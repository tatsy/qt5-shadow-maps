#ifndef _SHADOW_MAPPING_WIDGET_H_
#define _SHADOW_MAPPING_WIDGET_H_

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

class ShadowMapsWidget : public QOpenGLWidget {
    Q_OBJECT

public:
    explicit ShadowMapsWidget(QWidget* parent = NULL);
    ~ShadowMapsWidget();

protected:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int width, int height) override;

    void mousePressEvent(QMouseEvent* ev) override;
    void mouseMoveEvent(QMouseEvent* ev) override;
    void mouseReleaseEvent(QMouseEvent* ev) override;
    void wheelEvent(QWheelEvent* ev) override;

    void keyPressEvent(QKeyEvent* ev) override;

private:
    // Private methods
    void shadowMapping(QMatrix4x4* depthMVP);
    void drawScene(const QMatrix4x4& depthMVP);

    QOpenGLShaderProgram* compileShader(const QString& vShaderFile, const QString& fShaderFile);
    void drawVBO(QOpenGLShaderProgram* const program, const VBO& vbo);

    void setTexture(QOpenGLTexture* texture, const QImage& image) const;

    // Private fields
    static const int SHADOW_MAP_SIZE = 1024;
    static const QVector3D LIGHT_POSITION;

    QOpenGLShaderProgram* renderShader;
    QOpenGLShaderProgram* shadowmapShader;
    VBO objectVBO;
    VBO floorVBO;
    QOpenGLFramebufferObject* depthFBO;
    QOpenGLTexture* depthTexture;
    QOpenGLTexture* normalTexture;
    QOpenGLTexture* positionTexture;
    QOpenGLTexture* albedoTexture;

    ArcballController* arcball;
    Camera camera;
};

#endif // _SHADOW_MAPPING_WIDGET_H_
