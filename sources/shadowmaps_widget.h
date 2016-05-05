#ifndef _SHADOW_MAPPING_WIDGET_H_
#define _SHADOW_MAPPING_WIDGET_H_

#include <memory>

#include <QtCore/qtimer.h>
#include <QtCore/qvector.h>
#include <QtWidgets/qwidget.h>
#include <QtWidgets/qopenglwidget.h>

#include <QtGui/qopenglfunctions.h>
#include <QtGui/qopenglshaderprogram.h>
#include <QtGui/qopenglframebufferobject.h>
#include <QtGui/qopengltexture.h>
#include <QtGui/qevent.h>

#include "vbo.h"
#include "arcball_controller.h"

struct Camera {
    QVector3D eye, look, up;
};

enum class ShadowMaps : int {
    SM = 0x00,
    RSM = 0x01
};

class ShadowMapsWidget : public QOpenGLWidget, protected QOpenGLFunctions {
    Q_OBJECT

public:
    explicit ShadowMapsWidget(QWidget* parent = NULL);
    ~ShadowMapsWidget();

    void setShadowMode(ShadowMaps mode);

protected:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int width, int height) override;

    void mousePressEvent(QMouseEvent* ev) override;
    void mouseMoveEvent(QMouseEvent* ev) override;
    void mouseReleaseEvent(QMouseEvent* ev) override;
    void wheelEvent(QWheelEvent* ev) override;

private:
    // Private methods
    void shadowMapping(QMatrix4x4* depthMVP);
    void imperfectShadowMapping(const std::vector<QVector3D>& vplPositions,
                                std::vector<QMatrix4x4>* vplMVP);
    void drawScene(const QMatrix4x4& depthMVP);

    std::unique_ptr<QOpenGLShaderProgram> compileShader(
        const QString& vShaderFile,
        const QString& fShaderFile,
        const QString& gShaderFile = "");
    void drawVBO(QOpenGLShaderProgram* const program, const VBO& vbo);

    void setTexture(QOpenGLTexture* texture, const QImage& image) const;

    // Private fields
    static const int SHADOW_MAP_SIZE = 1024;
    static const QVector3D LIGHT_POSITION;

    std::unique_ptr<QOpenGLShaderProgram> renderShader    = nullptr;
    std::unique_ptr<QOpenGLShaderProgram> shadowmapShader = nullptr;
    std::unique_ptr<QOpenGLShaderProgram> ismShader = nullptr;

    VBO objectVBO;
    VBO floorVBO;
    std::unique_ptr<QOpenGLFramebufferObject> depthFBO    = nullptr;
    std::unique_ptr<QOpenGLFramebufferObject> positionFBO = nullptr;
    std::unique_ptr<QOpenGLFramebufferObject> normalFBO   = nullptr;
    std::unique_ptr<QOpenGLFramebufferObject> albedoFBO   = nullptr;

    std::unique_ptr<ArcballController> arcball = nullptr;
    Camera camera;

    ShadowMaps shadowMode = ShadowMaps::SM;
};

#endif // _SHADOW_MAPPING_WIDGET_H_
