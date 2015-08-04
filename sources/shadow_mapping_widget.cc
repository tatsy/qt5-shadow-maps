#include <cstdlib>
#include <iostream>

#include <QtGui/qmatrix4x4.h>

#include "shadow_mapping_widget.h"
#include "common.h"

namespace {

    QMatrix4x4 biasMat(
        0.5f, 0.0f, 0.0f, 0.5f,
        0.0f, 0.5f, 0.0f, 0.5f,
        0.0f, 0.0f, 0.5f, 0.5f,
        0.0f, 0.0f, 0.0f, 1.0f
    );

}

const QVector3D ShadowMappingWidget::CAMERA_POSITION = QVector3D(10.0f, 10.0f, 10.0f);
const QVector3D ShadowMappingWidget::LIGHT_POSITION = QVector3D(-10.0f, 10.0f, 10.0f);

ShadowMappingWidget::ShadowMappingWidget(QWidget* parent)
    : QOpenGLWidget(parent)
    , QOpenGLFunctions()
    , renderShader(NULL)
    , shadowmapShader(NULL)
    , objectVBO()
    , floorVBO()
    , depthFBO(NULL)
    , depthTexture(NULL)
{
    setWindowTitle("Qt5 shadow mapping");
}

ShadowMappingWidget::~ShadowMappingWidget()
{
    makeCurrent();

    delete renderShader;
    delete shadowmapShader;
    delete depthFBO;
    delete depthTexture;

    doneCurrent();
}

void ShadowMappingWidget::initializeGL() {
    initializeOpenGLFunctions();

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_NORMALIZE);
    glEnable(GL_TEXTURE_2D);

    // Load obj files
    objectVBO = VBO::fromObjFile((QString(DATA_DIRECTORY) + "bunny.obj").toStdString().c_str());
    floorVBO  = VBO::checkerFloor(QVector3D(0.0f, 1.0f, 0.0f), 5.0, 5.0, 32, 32);

    // Prepare shader programs
    renderShader    = compileShader(QString(SOURCE_DIRECTORY) + "render.vs", QString(SOURCE_DIRECTORY) + "render.fs");
    shadowmapShader = compileShader(QString(SOURCE_DIRECTORY) + "shadow_mapping.vs", QString(SOURCE_DIRECTORY) + "shadow_mapping.fs");

    // Initialize FBO
    depthFBO     = new QOpenGLFramebufferObject(SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, QOpenGLFramebufferObject::Attachment::Depth, GL_TEXTURE_2D);
    depthTexture = new QOpenGLTexture(QOpenGLTexture::Target::Target2D);
}

void ShadowMappingWidget::paintGL() {
    // Compute depth
    QMatrix4x4 depthMVP;
    shadowMapping(&depthMVP);
    
    // Draw scene
    drawScene(depthMVP);
}

void ShadowMappingWidget::resizeGL(int width, int height) {
    glViewport(0, 0, width, height);
}

void ShadowMappingWidget::drawScene(const QMatrix4x4& depthMVP) {
    // Set projection / modelview matrices
    QMatrix4x4 projectionMatrix, modelviewMatrix, normalMatrix;

    projectionMatrix.perspective(45.0f, (float)width() / (float)height(), 1.0f, 1000.0f);

    modelviewMatrix.lookAt(CAMERA_POSITION,               // Camera position
                           QVector3D(0.0f, 0.0f, 0.0f),   // Look at
                           QVector3D(0.0f, 1.0f, 0.0f));  // Up vector

    normalMatrix = modelviewMatrix.transposed().inverted();

    QMatrix4x4 depthBiasMVP = biasMat * depthMVP;

    // Set uniform variables
    makeCurrent();
    renderShader->bind();
    depthTexture->bind();

    renderShader->setUniformValue("shadowmap", 0);

    renderShader->setUniformValue("projectionMatrix", projectionMatrix);
    renderShader->setUniformValue("modelviewMatrix", modelviewMatrix);
    renderShader->setUniformValue("normalMatrix", normalMatrix);
    renderShader->setUniformValue("depthBiasMVP", depthBiasMVP);

    renderShader->setUniformValue("cameraPosition", CAMERA_POSITION);
    renderShader->setUniformValue("lightPosition", LIGHT_POSITION);

    // Draw scene
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    renderShader->setUniformValue("receiveShadow", 0);
    drawVBO(renderShader, objectVBO);
    renderShader->setUniformValue("receiveShadow", 1);
    drawVBO(renderShader, floorVBO);

    renderShader->release();
    depthTexture->release();
}

void ShadowMappingWidget::shadowMapping(QMatrix4x4* depthMVP) {
    QMatrix4x4 projectionMatrix, modelviewMatrix;

    glViewport(0, 0, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);

    depthFBO->bind();
    shadowmapShader->bind();

    projectionMatrix.ortho(-10.0f, 10.0f, -10.0f, 10.0f, 0.0f, 50.0f);
    modelviewMatrix.lookAt(LIGHT_POSITION,
                           QVector3D(0.0f, 0.0f, 0.0f),
                           QVector3D(0.0f, 1.0f, 0.0f));

    *depthMVP = projectionMatrix * modelviewMatrix;

    shadowmapShader->setUniformValue("modelviewProjectionMatrix", *depthMVP);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    
    drawVBO(shadowmapShader, objectVBO);

    const QImage depthImage = depthFBO->toImage().mirrored();

    depthTexture->destroy();
    depthTexture->create();
    depthTexture->setData(depthImage, QOpenGLTexture::MipMapGeneration::GenerateMipMaps);
    depthTexture->setMinificationFilter(QOpenGLTexture::Filter::Nearest);
    depthTexture->setMagnificationFilter(QOpenGLTexture::Filter::Nearest);
    depthTexture->setWrapMode(QOpenGLTexture::CoordinateDirection::DirectionS, QOpenGLTexture::WrapMode::ClampToEdge);
    depthTexture->setWrapMode(QOpenGLTexture::CoordinateDirection::DirectionT, QOpenGLTexture::WrapMode::ClampToEdge);

    depthFBO->release();
    shadowmapShader->release();

    glViewport(0, 0, width(), height());
}

void ShadowMappingWidget::drawVBO(QOpenGLShaderProgram* const shader, const VBO& vbo) {
    shader->enableAttributeArray("vertices");
    shader->enableAttributeArray("normals");
    shader->enableAttributeArray("colors");

    shader->setAttributeArray("vertices", &vbo.vertices()[0]);
    shader->setAttributeArray("normals", &vbo.normals()[0]);
    shader->setAttributeArray("colors", &vbo.colors()[0]);

    glDrawElements(GL_TRIANGLES, vbo.indices().size() * 3, GL_UNSIGNED_INT, (unsigned int*)&vbo.indices()[0]);

    shader->disableAttributeArray("vertices");
    shader->disableAttributeArray("normals");
    shader->disableAttributeArray("colors");
}

QOpenGLShaderProgram* ShadowMappingWidget::compileShader(const QString& vShaderFile, const QString& fShaderFile) {
    QOpenGLShaderProgram* shader = new QOpenGLShaderProgram();
    shader->addShaderFromSourceFile(QOpenGLShader::Vertex, vShaderFile);
    shader->addShaderFromSourceFile(QOpenGLShader::Fragment, fShaderFile);
    shader->link();

    if (!shader->isLinked()) {
        std::cerr << "Failed to link shaders: " << std::endl;
        std::cerr << "  Vertex: " << vShaderFile.toStdString() << std::endl;
        std::cerr << "Fragment: " << fShaderFile.toStdString() << std::endl;
        exit(1);
    }

    return shader;
}

