#include <cstdlib>
#include <ctime>
#include <iostream>
#include <vector>

#include <QtGui/qmatrix4x4.h>

#include "shadowmaps_widget.h"
#include "common.h"

namespace {

QMatrix4x4 biasMat(
    0.5f, 0.0f, 0.0f, 0.5f,
    0.0f, 0.5f, 0.0f, 0.5f,
    0.0f, 0.0f, 0.5f, 0.5f,
    0.0f, 0.0f, 0.0f, 1.0f
);

enum class DrawMode : int {
    Depth = 0x00,
    Normal = 0x01,
    Position = 0x02,
    Albedo = 0x03
};

void showInfo() {
    std::cout << " ----- OpenGL Support -----" << std::endl;
    std::cout << "        Vendor: " << glGetString(GL_VENDOR) << std::endl;
    std::cout << "      Renderer: " << glGetString(GL_RENDERER) << std::endl;
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "  GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl << std::endl;
}

}  // anonymous namespace


const QVector3D ShadowMapsWidget::LIGHT_POSITION = QVector3D(2.0f, 8.0f, 10.0f);

ShadowMapsWidget::ShadowMapsWidget(QWidget* parent)
    : QOpenGLWidget(parent)
    , renderShader(NULL)
    , shadowmapShader(NULL)
    , objectVBO()
    , floorVBO()
    , depthFBO(NULL)
    , depthTexture(NULL)
    , arcball(NULL) {
    setWindowTitle("Qt5 Shadow Maps (SM)");

    arcball = new ArcballController(this);
    camera.eye = QVector3D(10.0f, 10.0f, 10.0f);
    camera.look = QVector3D(0.0f, 0.0f, 0.0f);
    camera.up = QVector3D(0.0f, 1.0f, 0.0f);
}

ShadowMapsWidget::~ShadowMapsWidget() {
    makeCurrent();

    delete renderShader;
    delete shadowmapShader;
    delete depthFBO;
    delete depthTexture;
    delete arcball;

    doneCurrent();
}

void ShadowMapsWidget::setShadowMode(ShadowMaps mode) {
    shadowMode = mode;
}

void ShadowMapsWidget::initializeGL() {
    showInfo();

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_NORMALIZE);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_CULL_FACE);

    // Load obj files
    objectVBO = VBO::fromObjFile((QString(DATA_DIRECTORY) + "budda.obj").toStdString().c_str(), QVector3D(0.5f, 0.5f, 0.5f));
    floorVBO  = VBO::colorBox(); 

    // Prepare shader programs
    renderShader    = compileShader(QString(SOURCE_DIRECTORY) + "render.vs", QString(SOURCE_DIRECTORY) + "render.fs");
    shadowmapShader = compileShader(QString(SOURCE_DIRECTORY) + "shadow_mapping.vs", QString(SOURCE_DIRECTORY) + "shadow_mapping.fs");

    // Initialize FBO
    depthFBO        = new QOpenGLFramebufferObject(SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, QOpenGLFramebufferObject::Attachment::Depth, GL_TEXTURE_2D);
    depthTexture    = new QOpenGLTexture(QOpenGLTexture::Target::Target2D);
    normalTexture   = new QOpenGLTexture(QOpenGLTexture::Target::Target2D);
    positionTexture = new QOpenGLTexture(QOpenGLTexture::Target::Target2D);
    albedoTexture   = new QOpenGLTexture(QOpenGLTexture::Target::Target2D);

    // Initialize arcball controller
    QMatrix4x4 modelMat, viewMat;
    modelMat.setToIdentity();
    viewMat.lookAt(camera.eye, camera.look, camera.up);
    arcball->initModelView(modelMat, viewMat);
}

void ShadowMapsWidget::paintGL() {
    // Compute shadow maps   
    static QMatrix4x4 depthMVP;
    if (depthMVP.isIdentity()) {
        shadowMapping(&depthMVP);
    }

    // Draw scene
    drawScene(depthMVP);
}

void ShadowMapsWidget::resizeGL(int width, int height) {
    glViewport(0, 0, width, height);
}

void ShadowMapsWidget::mousePressEvent(QMouseEvent* ev) {
    arcball->setOldPoint(ev->pos());
    arcball->setNewPoint(ev->pos());
    if (ev->button() == Qt::LeftButton) {
        arcball->setMode(ArcballMode::Rotate);
    } else if (ev->button() == Qt::RightButton) {
        arcball->setMode(ArcballMode::Translate);
    } else if (ev->button() == Qt::MiddleButton) {
        arcball->setMode(ArcballMode::Scale);
    }
}

void ShadowMapsWidget::mouseMoveEvent(QMouseEvent* ev) {
    arcball->setNewPoint(ev->pos());
    arcball->update();
    update();
    arcball->setOldPoint(ev->pos());
}

void ShadowMapsWidget::mouseReleaseEvent(QMouseEvent* ev) {
    arcball->setMode(ArcballMode::None);
}

void ShadowMapsWidget::wheelEvent(QWheelEvent* ev) {
    arcball->setScroll(arcball->scroll() + ev->delta() / 1000.0);
    arcball->update();
    update();
}

void ShadowMapsWidget::drawScene(const QMatrix4x4& depthMVP) {
    // Set projection / modelview matrices
    QMatrix4x4 projectionMatrix, modelviewMatrix, normalMatrix;

    projectionMatrix.perspective(45.0f, (float)width() / (float)height(), 1.0f, 1000.0f);

    modelviewMatrix = arcball->modelviewMatrix();
    normalMatrix = modelviewMatrix.transposed().inverted();

    QMatrix4x4 depthBiasMVP = biasMat * depthMVP;

    // Set uniform variables
    makeCurrent();
    renderShader->bind();
    depthTexture->bind(0);
    normalTexture->bind(1);
    positionTexture->bind(2);
    albedoTexture->bind(3);

    // Compute sampling pattern
    const int nSamples = 256;
    static std::vector<QVector2D> randoms;
    if (randoms.empty()) {
        randoms.resize(nSamples);
        srand((unsigned long)time(0));
        for (int i = 0; i < 256; i++) {
            double r0 = rand() / (float)RAND_MAX;
            double r1 = rand() / (float)RAND_MAX;
            randoms[i] = QVector2D(r0 * cos(2.0 * M_PI * r1), r0 * sin(2.0 * M_PI * r1));
        }
    }

    renderShader->setUniformValue("depthMap", 0);
    renderShader->setUniformValue("normalMap", 1);
    renderShader->setUniformValue("positionMap", 2);
    renderShader->setUniformValue("albedoMap", 3);

    renderShader->setUniformValue("projectionMatrix", projectionMatrix);
    renderShader->setUniformValue("modelviewMatrix", modelviewMatrix);
    renderShader->setUniformValue("normalMatrix", normalMatrix);
    renderShader->setUniformValue("depthBiasMVP", depthBiasMVP);

    renderShader->setUniformValue("cameraPosition", camera.eye);
    renderShader->setUniformValue("lightPosition", LIGHT_POSITION);

    renderShader->setUniformValue("nSamples", nSamples);
    renderShader->setUniformValueArray("delta", &randoms[0], 256);

    // Draw scene
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    renderShader->setUniformValue("mode", (int)shadowMode);

    renderShader->setUniformValue("receiveShadow", 0);
    drawVBO(renderShader, objectVBO);
    renderShader->setUniformValue("receiveShadow", 1);
    drawVBO(renderShader, floorVBO);

    renderShader->release();
    depthTexture->release();
    normalTexture->release();
    positionTexture->release();
    albedoTexture->release();
}

void ShadowMapsWidget::shadowMapping(QMatrix4x4* depthMVP) {
    QMatrix4x4 projectionMatrix, modelviewMatrix, normalMatrix;

    glViewport(0, 0, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);

    depthFBO->bind();
    shadowmapShader->bind();

    projectionMatrix.ortho(-10.0f, 10.0f, -10.0f, 10.0f, 0.0f, 25.0f);
    modelviewMatrix.lookAt(LIGHT_POSITION,
                           QVector3D(0.0f, 0.0f, 0.0f),
                           QVector3D(0.0f, 1.0f, 0.0f));

    *depthMVP = projectionMatrix * modelviewMatrix;
    normalMatrix = modelviewMatrix.transposed().inverted();

    shadowmapShader->setUniformValue("projectionMatrix", projectionMatrix);
    shadowmapShader->setUniformValue("modelviewMatrix", modelviewMatrix);
    shadowmapShader->setUniformValue("normalMatrix", normalMatrix);
    shadowmapShader->setUniformValue("cameraPosition", LIGHT_POSITION);
    
    // Depth
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    shadowmapShader->setUniformValue("mode", (int)DrawMode::Depth);
    drawVBO(shadowmapShader, objectVBO);
    //drawVBO(shadowmapShader, floorVBO);

    QImage depthImage = depthFBO->toImage();
    depthImage.save("depth.png");
    setTexture(depthTexture, depthImage);

    // Normal
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    shadowmapShader->setUniformValue("mode", (int)DrawMode::Normal);
    //drawVBO(shadowmapShader, objectVBO);
    drawVBO(shadowmapShader, floorVBO);
    
    QImage normalImage = depthFBO->toImage();
    normalImage.save("normal.png");
    setTexture(normalTexture, normalImage);

    // Position
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    shadowmapShader->setUniformValue("mode", (int)DrawMode::Position);
    //drawVBO(shadowmapShader, objectVBO);
    drawVBO(shadowmapShader, floorVBO);

    QImage positionImage = depthFBO->toImage();
    positionImage.save("position.png");
    setTexture(positionTexture, positionImage);
    
    // Albedo
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    shadowmapShader->setUniformValue("mode", (int)DrawMode::Albedo);
    //drawVBO(shadowmapShader, objectVBO);
    drawVBO(shadowmapShader, floorVBO);
    
    QImage albedoImage = depthFBO->toImage();
    albedoImage.save("albedo.png");
    setTexture(albedoTexture, albedoImage);

    depthFBO->release();
    shadowmapShader->release();

    glViewport(0, 0, width(), height());
}

void ShadowMapsWidget::drawVBO(QOpenGLShaderProgram* const shader, const VBO& vbo) {
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

void ShadowMapsWidget::setTexture(QOpenGLTexture* texture, const QImage& image) const {
    const int width  = image.width();
    const int height = image.height();
    QImage mirrored = image;

    for (int y = 0; y < height / 2; y++) {
        for (int x = 0; x < width; x++) {
            QRgb rgb = mirrored.pixel(x, y);
            mirrored.setPixel(x, y, mirrored.pixel(x, height - y - 1));
            mirrored.setPixel(x, height - y - 1, rgb);
        }
    }

    texture->destroy();
    texture->create();
    texture->setData(mirrored, QOpenGLTexture::MipMapGeneration::GenerateMipMaps);
    texture->setMinificationFilter(QOpenGLTexture::Filter::Linear);
    texture->setMagnificationFilter(QOpenGLTexture::Filter::Linear);
    texture->setWrapMode(QOpenGLTexture::CoordinateDirection::DirectionS, QOpenGLTexture::WrapMode::ClampToEdge);
    texture->setWrapMode(QOpenGLTexture::CoordinateDirection::DirectionT, QOpenGLTexture::WrapMode::ClampToEdge);

}

QOpenGLShaderProgram* ShadowMapsWidget::compileShader(const QString& vShaderFile, const QString& fShaderFile) {
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

