#include <cstdlib>
#include <ctime>
#include <iostream>
#include <vector>

#include <QtGui/qmatrix4x4.h>

#include "shadowmaps_widget.h"
#include "common.h"

namespace {

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


const QVector3D ShadowMapsWidget::LIGHT_POSITION = QVector3D(0.0f, 4.9f, 0.0f);

ShadowMapsWidget::ShadowMapsWidget(QWidget* parent)
    : QOpenGLWidget(parent)
    , QOpenGLFunctions()
    , objectVBO()
    , floorVBO() {
    setWindowTitle("Qt5 Shadow Maps (SM)");

    arcball = std::make_unique<ArcballController>(this);
    camera.eye  = QVector3D(0.0f, 0.0f, 15.0f);
    camera.look = QVector3D(0.0f, 0.0f, 0.0f);
    camera.up   = QVector3D(0.0f, 1.0f, 0.0f);
}

ShadowMapsWidget::~ShadowMapsWidget() {
    makeCurrent();
    doneCurrent();
}

void ShadowMapsWidget::setShadowMode(ShadowMaps mode) {
    shadowMode = mode;
}

void ShadowMapsWidget::initializeGL() {
    showInfo();

    initializeOpenGLFunctions();

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_NORMALIZE);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_CULL_FACE);

    // Load obj files
    const char* objfile = (std::string(DATA_DIRECTORY) + "budda.obj").c_str();
    objectVBO = VBO::fromObjFile(objfile, QVector3D(0.5f, 0.5f, 0.5f));
    floorVBO  = VBO::colorBox(); 

    // Prepare shader programs
    renderShader    = compileShader(QString(SOURCE_DIRECTORY) + "shaders/render.vs",
                                    QString(SOURCE_DIRECTORY) + "shaders/render.fs");
    shadowmapShader = compileShader(QString(SOURCE_DIRECTORY) + "shaders/shadow_maps.vs",
                                    QString(SOURCE_DIRECTORY) + "shaders/shadow_maps.fs");
    depthShader     = compileShader(QString(SOURCE_DIRECTORY) + "shaders/depth.vs",
                                    QString(SOURCE_DIRECTORY) + "shaders/depth.fs");
    normalShader    = compileShader(QString(SOURCE_DIRECTORY) + "shaders/normal.vs",
                                    QString(SOURCE_DIRECTORY) + "shaders/normal.fs");
    ismShader       = compileShader(QString(SOURCE_DIRECTORY) + "shaders/ism.vs",
                                    QString(SOURCE_DIRECTORY) + "shaders/ism.fs",
                                    QString(SOURCE_DIRECTORY) + "shaders/ism.gs");
    ismRenderShader = compileShader(QString(SOURCE_DIRECTORY) + "shaders/ismrender.vs",
                                    QString(SOURCE_DIRECTORY) + "shaders/ismrender.fs");

    // Initialize FBO
    depthFBO = std::make_unique<QOpenGLFramebufferObject>(
        SHADOW_MAP_SIZE, SHADOW_MAP_SIZE,
        QOpenGLFramebufferObject::Attachment::Depth,
        GL_TEXTURE_2D, GL_RGBA32F);
    positionFBO = std::make_unique<QOpenGLFramebufferObject>(
        SHADOW_MAP_SIZE, SHADOW_MAP_SIZE,
        QOpenGLFramebufferObject::Attachment::Depth,
        GL_TEXTURE_2D, GL_RGBA32F);
    normalFBO = std::make_unique<QOpenGLFramebufferObject>(
        SHADOW_MAP_SIZE, SHADOW_MAP_SIZE,
        QOpenGLFramebufferObject::Attachment::Depth,
        GL_TEXTURE_2D, GL_RGBA32F);
    albedoFBO = std::make_unique<QOpenGLFramebufferObject>(
        SHADOW_MAP_SIZE, SHADOW_MAP_SIZE,
        QOpenGLFramebufferObject::Attachment::Depth,
        GL_TEXTURE_2D, GL_RGBA32F);

    // Initialize arcball controller
    QMatrix4x4 modelMat, viewMat;
    modelMat.setToIdentity();
    viewMat.lookAt(camera.eye, camera.look, camera.up);
    arcball->initModelView(modelMat, viewMat);
}

void ShadowMapsWidget::paintGL() {
    // Compute shadow maps   
    static QMatrix4x4 depthMVP;
    static std::vector<VPL> vpls;
    if (depthMVP.isIdentity()) {
        shadowMapping(&depthMVP);
        imperfectShadowMapping(&vpls);
    }

    // Draw ISM
    if (shadowMode == ShadowMaps::ISM) {
        drawISM(vpls);
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

    // Set uniform variables
    makeCurrent();
    renderShader->bind();

    // Compute sampling pattern
    const int nSamples = 256;
    static std::vector<QVector2D> randoms;
    if (randoms.empty()) {
        randoms.resize(nSamples);
        srand((unsigned long)time(0));
        for (int i = 0; i < nSamples; i++) {
            double r0 = rand() / (float)RAND_MAX;
            double r1 = rand() / (float)RAND_MAX;
            randoms[i] = QVector2D(r0 * cos(2.0 * M_PI * r1), r0 * sin(2.0 * M_PI * r1));
        }
    }

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, depthFBO->texture());

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, normalFBO->texture());

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, positionFBO->texture());

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, albedoFBO->texture());

    if (shadowMode == ShadowMaps::ISM) {
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, indirectFBO->texture());
    }

    renderShader->setUniformValue("depthMap",    0);
    renderShader->setUniformValue("normalMap",   1);
    renderShader->setUniformValue("positionMap", 2);
    renderShader->setUniformValue("albedoMap",   3);
    renderShader->setUniformValue("indirectMap", 4);

    renderShader->setUniformValue("projectionMatrix", projectionMatrix);
    renderShader->setUniformValue("modelviewMatrix", modelviewMatrix);
    renderShader->setUniformValue("normalMatrix", normalMatrix);
    renderShader->setUniformValue("depthMVP", depthMVP);

    renderShader->setUniformValue("Le", QVector3D(4.0f, 4.0f, 4.0f));
    renderShader->setUniformValue("cameraPosition", camera.eye);
    renderShader->setUniformValue("lightPosition", LIGHT_POSITION);

    renderShader->setUniformValue("nSamples", nSamples);
    renderShader->setUniformValueArray("delta", &randoms[0], nSamples);

    // Draw scene
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    renderShader->setUniformValue("mode", (int)shadowMode);

    renderShader->setUniformValue("receiveShadow", 0);
    drawVBO(renderShader.get(), objectVBO);
    renderShader->setUniformValue("receiveShadow", 1);
    drawVBO(renderShader.get(), floorVBO);

    renderShader->release();
}

void ShadowMapsWidget::drawISM(const std::vector<VPL>& VPLs) {
    QOpenGLFramebufferObject renderFBO1(width(), height(),
        QOpenGLFramebufferObject::Attachment::Depth, GL_TEXTURE_2D, GL_RGBA32F);
    QOpenGLFramebufferObject renderFBO2(width(), height(),
        QOpenGLFramebufferObject::Attachment::Depth, GL_TEXTURE_2D, GL_RGBA32F);
    indirectFBO = std::make_unique<QOpenGLFramebufferObject>(width(), height(),
        QOpenGLFramebufferObject::Attachment::Depth, GL_TEXTURE_2D, GL_RGBA32F);

    QMatrix4x4 projMat, mvMat, mvpMat;
    projMat.perspective(45.0f, (float)width() / (float)height(), 1.0f, 1000.0f);
    mvMat = arcball->modelviewMatrix();
    mvpMat = projMat * mvMat;

    const int nVPL = (int)VPLs.size();

    makeCurrent();
    ismRenderShader->bind();

    glViewport(0, 0, width(), height());

    const int ismCols = 32;
    const int ismRows = 32;

    std::vector<QVector3D> posVPLs(ismCols);
    std::vector<QVector3D> nrmVPLs(ismCols);
    std::vector<QVector3D> albVPLs(ismCols);
    std::vector<QMatrix4x4> mvVPLs(ismCols);

    renderFBO1.bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    renderFBO1.release();

    renderFBO2.bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    renderFBO2.release();

    indirectFBO->bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    indirectFBO->release();

    int currentRow = 0;
    for (int i = 0; i < nVPL; i += ismCols, currentRow++) {
        if (currentRow % 2 == 0) {
            if (currentRow == ismRows - 1) {
                indirectFBO->bind();
            } else {
                renderFBO1.bind();
            }
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, ismFBO->texture());
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, renderFBO2.texture());
        } else {
            if (currentRow == ismRows - 1) {
                indirectFBO->bind();
            } else {
                renderFBO2.bind();
            }
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, ismFBO->texture());
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, renderFBO1.texture());        
        }

        ismRenderShader->setUniformValue("mvpMat", mvpMat);
        ismRenderShader->setUniformValue("lightPosition", LIGHT_POSITION);
        ismRenderShader->setUniformValue("currentRow", currentRow);
        ismRenderShader->setUniformValue("ismRows", ismRows);
        ismRenderShader->setUniformValue("ismCols", ismCols);
        ismRenderShader->setUniformValue("maxDepth", 20.0f);
        ismRenderShader->setUniformValue("ismTexture", 0);
        ismRenderShader->setUniformValue("accumTexture", 1);

        for (int j = 0; j < ismCols; j++) {
            posVPLs[j] = VPLs[j].pos;
            nrmVPLs[j] = VPLs[j].nrm;
            albVPLs[j] = VPLs[j].albedo;
            mvVPLs[j] = VPLs[j].mvMat;
        }

        ismRenderShader->setUniformValueArray("posVPL", &posVPLs[0], ismCols);
        ismRenderShader->setUniformValueArray("nrmVPL", &nrmVPLs[0], ismCols);
        ismRenderShader->setUniformValueArray("albVPL", &albVPLs[0], ismCols);
        ismRenderShader->setUniformValueArray("mvVPL", &mvVPLs[0], ismCols);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        drawVBO(ismRenderShader.get(), objectVBO);
        drawVBO(ismRenderShader.get(), floorVBO);

        if (currentRow == ismRows - 1) {
            indirectFBO->release();
        } else {
            if (currentRow % 2 == 0) {
                renderFBO1.release();
            } else {
                renderFBO2.release();
            }
        }
    }

    ismRenderShader->release();
    indirectFBO->toImage().save(QString(OUTPUT_DIRECTORY) + "indirect.png");
}

void ShadowMapsWidget::shadowMapping(QMatrix4x4* depthMVP) {
    QMatrix4x4 projectionMatrix, modelviewMatrix, normalMatrix;

    glViewport(0, 0, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);

    shadowmapShader->bind();

    projectionMatrix.perspective(90.0f, 1.0f, 0.1f, 100.0f);
    modelviewMatrix.lookAt(LIGHT_POSITION,
                           QVector3D(0.0f, 0.0f, 0.0f),
                           QVector3D(1.0f, 0.0f, 0.0f));

    *depthMVP = projectionMatrix * modelviewMatrix;
    normalMatrix = modelviewMatrix.transposed().inverted();

    shadowmapShader->setUniformValue("projectionMatrix", projectionMatrix);
    shadowmapShader->setUniformValue("modelviewMatrix", modelviewMatrix);
    shadowmapShader->setUniformValue("normalMatrix", normalMatrix);
    shadowmapShader->setUniformValue("cameraPosition", LIGHT_POSITION);
    
    // Depth
    depthFBO->bind();
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    shadowmapShader->setUniformValue("mode", (int)DrawMode::Depth);
    drawVBO(shadowmapShader.get(), objectVBO);
    depthFBO->release();
    //drawVBO(shadowmapShader, floorVBO);

    QImage depthImage = depthFBO->toImage();
    depthImage.save(QString(OUTPUT_DIRECTORY) + "depth.png");

    // Normal
    normalFBO->bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    shadowmapShader->setUniformValue("mode", (int)DrawMode::Normal);
    //drawVBO(shadowmapShader, objectVBO);
    drawVBO(shadowmapShader.get(), floorVBO);
    normalFBO->release();
    
    QImage normalImage = normalFBO->toImage();
    normalImage.save(QString(OUTPUT_DIRECTORY) + "normal.png");

    // Position
    positionFBO->bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    shadowmapShader->setUniformValue("mode", (int)DrawMode::Position);
    //drawVBO(shadowmapShader, objectVBO);
    drawVBO(shadowmapShader.get(), floorVBO);
    positionFBO->release();

    QImage positionImage = positionFBO->toImage();
    positionImage.save(QString(OUTPUT_DIRECTORY) + "position.png");
    
    // Albedo
    albedoFBO->bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    shadowmapShader->setUniformValue("mode", (int)DrawMode::Albedo);
    //drawVBO(shadowmapShader, objectVBO);
    drawVBO(shadowmapShader.get(), floorVBO);
    albedoFBO->release();
    
    QImage albedoImage = albedoFBO->toImage();
    albedoImage.save(QString(OUTPUT_DIRECTORY) + "albedo.png");

    shadowmapShader->release();

    glViewport(0, 0, width(), height());
}

void ShadowMapsWidget::imperfectShadowMapping(
    std::vector<VPL>* vpls) {

    static const int nVPL = 1024;
    static const double maxArea = 0.01;

    // Prepare point cloud
    std::vector<QVector3D> vertices;
    std::vector<unsigned int> indices;
    srand((unsigned long)time(0));
    for (const auto& t : objectVBO.indices()) {
        QVector3D v0 = objectVBO.vertices()[t.i];
        QVector3D v1 = objectVBO.vertices()[t.j];
        QVector3D v2 = objectVBO.vertices()[t.k];
        double area = 0.5 * QVector3D::crossProduct(v1 - v0, v2 - v0).length();
        const int nv = std::max(1, (int)(area / maxArea + 0.5));
        for (int i = 0; i < nv; i++) {
            double r1 = (double)rand() / RAND_MAX;            
            double r2 = (double)rand() / RAND_MAX;
            if (r1 + r2 > 1.0) {
                r1 = 1.0 - r1;
                r2 = 1.0 - r2;
            }

            QVector3D pos = (1.0 - r1 - r2) * v0 + r1 * v1 + r2 * v2;
            vertices.push_back(pos);
            indices.push_back(indices.size());
        }
    }

    for (const auto& t : floorVBO.indices()) {
        QVector3D v0 = floorVBO.vertices()[t.i];
        QVector3D v1 = floorVBO.vertices()[t.j];
        QVector3D v2 = floorVBO.vertices()[t.k];
        double area = 0.5 * QVector3D::crossProduct(v1 - v0, v2 - v0).length();
        const int nv = std::max(1, (int)(area / maxArea + 0.5));
        for (int i = 0; i < nv; i++) {
            double r1 = (double)rand() / RAND_MAX;            
            double r2 = (double)rand() / RAND_MAX;
            if (r1 + r2 > 1.0) {
                r1 = 1.0 - r1;
                r2 = 1.0 - r2;
            }

            QVector3D pos = (1.0 - r1 - r2) * v0 + r1 * v1 + r2 * v2;
            vertices.push_back(pos);
            indices.push_back(indices.size());
        }
    }

    QOpenGLFramebufferObject fbo(SHADOW_MAP_SIZE, SHADOW_MAP_SIZE,
        QOpenGLFramebufferObject::Attachment::Depth, GL_TEXTURE_2D);

    // Compute ordinary position/normal map for VPL collection
    glViewport(0, 0, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);

    QMatrix4x4 projMat, mvMat, mvpMat, normalMat;
    projMat.perspective(85.0f, 1.0f, 0.1f, 100.0f);
    mvMat.lookAt(LIGHT_POSITION, QVector3D(0.0, 0.0, 0.0), QVector3D(1.0, 0.0, 0.0));

    mvpMat = projMat * mvMat;
    normalMat = mvMat.transposed().inverted();

    QImage depthImage;
    {
        depthShader->bind();
        fbo.bind();

        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

        depthShader->setUniformValue("mvpMat", mvpMat);

        drawVBO(depthShader.get(), floorVBO);
        drawVBO(depthShader.get(), objectVBO);

        depthImage = fbo.toImage();
        depthImage.save(QString(OUTPUT_DIRECTORY) + "depthISM.png");

        depthShader->release();
        fbo.release();
    }

    QImage normalImage;
    {
        shadowmapShader->bind();
        fbo.bind();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shadowmapShader->setUniformValue("mvpMat", mvpMat);
        shadowmapShader->setUniformValue("mode", (int)DrawMode::Normal);

        drawVBO(shadowmapShader.get(), floorVBO);
        drawVBO(shadowmapShader.get(), objectVBO);

        normalImage = fbo.toImage();
        normalImage.save(QString(OUTPUT_DIRECTORY) + "normalISM.png");

        shadowmapShader->release();
        fbo.release();
    }

    QImage albedoImage;
    {
        shadowmapShader->bind();
        fbo.bind();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shadowmapShader->setUniformValue("mvpMat", mvpMat);
        shadowmapShader->setUniformValue("mode", (int)DrawMode::Albedo);

        drawVBO(shadowmapShader.get(), floorVBO);
        drawVBO(shadowmapShader.get(), objectVBO);

        albedoImage = fbo.toImage();
        albedoImage.save(QString(OUTPUT_DIRECTORY) + "albedoISM.png");

        shadowmapShader->release();
        fbo.release();
    }

    std::vector<QVector3D> vplPos;
    std::vector<QVector3D> vplNrm;
    std::vector<QVector3D> vplAlb;
    while (vplPos.size() < nVPL) {
        const int rx = rand() % SHADOW_MAP_SIZE;
        const int ry = rand() % SHADOW_MAP_SIZE;
        const double px = 2.0 * rx / SHADOW_MAP_SIZE - 1.0;
        const double py = 1.0 - 2.0 * ry / SHADOW_MAP_SIZE;

        QColor rgb;
        rgb = QColor::fromRgba(depthImage.pixel(rx, ry));
        const double pz = rgb.redF() + rgb.greenF() / 255.0 + rgb.blueF() / (255.0 * 255.0);
        if (pz > 0.999) {
            continue;
        }

        rgb = QColor::fromRgba(normalImage.pixel(rx, ry));
        
        QVector3D pos(px, py, pz);
        pos = mvpMat.inverted() * pos;
        QVector3D nrm = QVector3D(2.0 * rgb.redF() - 1.0, rgb.greenF() * 2.0 - 1.0, rgb.blueF() * 2.0 - 1.0);

        rgb = QColor::fromRgba(albedoImage.pixel(rx, ry));

        QVector3D alb = QVector3D(rgb.redF(), rgb.greenF(), rgb.blueF());

        vplPos.push_back(pos);
        vplNrm.push_back(nrm);
        vplAlb.push_back(alb);
    }

    // Compute SM from VPLs
    static const int ismRes = 128;
    static const int ismRows = 32;
    static const int ismCols = 32;
    ismFBO = std::make_unique<QOpenGLFramebufferObject>(
        ismRes * ismCols, ismRes * ismRows,
        QOpenGLFramebufferObject::Attachment::Depth, GL_TEXTURE_2D, GL_RGBA32F);

    ismShader->bind();
    ismFBO->bind();

    glViewport(0, 0, ismRes * ismCols, ismRes * ismRows);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    vpls->clear();
    std::vector<QMatrix4x4> mvVPLs(nVPL);
    for (int i = 0; i < nVPL; i++) {
        QVector3D up = std::abs(vplNrm[i].y()) < 0.1f ? QVector3D(0.0f, 1.0f, 0.0f) : QVector3D(1.0f, 0.0f, 0.0f);
        mvVPLs[i].lookAt(vplPos[i] + 0.05 * vplNrm[i], vplPos[i] + vplNrm[i], up);
        vpls->emplace_back(vplPos[i], vplNrm[i], vplAlb[i], mvVPLs[i]);
    }

    int currentRow = 0;
    for (int i = 0; i < nVPL; i += ismCols, currentRow++) {
        ismShader->setUniformValue("currentRow", currentRow);
        ismShader->setUniformValue("ismRows", ismRows);
        ismShader->setUniformValue("ismCols", ismCols);
        ismShader->setUniformValue("maxDepth", 20.0f);
        ismShader->setUniformValueArray("mvVPL", &mvVPLs[0] + i, ismCols);

        ismShader->setAttributeArray("vertices", &vertices[0]);
        ismShader->enableAttributeArray("vertices");

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glEnable(GL_POINT_SPRITE);
        glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
        glDrawElements(GL_POINTS, indices.size(), GL_UNSIGNED_INT, &indices[0]);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);
        glDisable(GL_POINT_SIZE);
        glDisable(GL_VERTEX_PROGRAM_POINT_SIZE);

        ismShader->disableAttributeArray("vertices");
    }
    
    ismFBO->toImage().save(QString(OUTPUT_DIRECTORY) + "ism.png");

    ismShader->release();
    ismFBO->release();

    glViewport(0, 0, width(), height());
}

void ShadowMapsWidget::drawVBO(QOpenGLShaderProgram* const shader, const VBO& vbo) {
    shader->enableAttributeArray("vertices");
    shader->enableAttributeArray("normals");
    shader->enableAttributeArray("colors");

    shader->setAttributeArray("vertices", &vbo.vertices()[0]);
    shader->setAttributeArray("normals", &vbo.normals()[0]);
    shader->setAttributeArray("colors", &vbo.colors()[0]);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glDrawElements(GL_TRIANGLES, vbo.indices().size() * 3, GL_UNSIGNED_INT, (unsigned int*)&vbo.indices()[0]);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

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

std::unique_ptr<QOpenGLShaderProgram> ShadowMapsWidget::compileShader(
    const QString& vShaderFile,
    const QString& fShaderFile,
    const QString& gShaderFile) {
    auto shader = std::make_unique<QOpenGLShaderProgram>();
    shader->addShaderFromSourceFile(QOpenGLShader::Vertex, vShaderFile);
    shader->addShaderFromSourceFile(QOpenGLShader::Fragment, fShaderFile);
    if (gShaderFile != "") {
        shader->addShaderFromSourceFile(QOpenGLShader::Geometry, gShaderFile);
    }
    shader->link();

    if (!shader->isLinked()) {
        std::cerr << "Failed to link shaders: " << std::endl;
        std::cerr << "  Vertex: " << vShaderFile.toStdString() << std::endl;
        std::cerr << "Fragment: " << fShaderFile.toStdString() << std::endl;
        exit(1);
    }

    return shader;
}

