#ifndef _SHADOW_MAPPING_WIDGET_H_
#define _SHADOW_MAPPING_WIDGET_H_

#include <QtCore/qtimer.h>
#include <QtWidgets/qwidget.h>
#include <QtWidgets/qopenglwidget.h>

#include <QtGui/qopenglfunctions.h>
#include <QtGui/qopenglshaderprogram.h>
#include <QtGui/qopenglframebufferobject.h>
#include <QtGui/qopengltexture.h>

#include "vbo.h"

class ShadowMappingWidget : public QOpenGLWidget, private QOpenGLFunctions {
    Q_OBJECT

private:
    static const int SHADOW_MAP_SIZE = 1024;
    static const QVector3D CAMERA_POSITION;
    static const QVector3D LIGHT_POSITION;

    QOpenGLShaderProgram* renderShader;
    QOpenGLShaderProgram* shadowmapShader;
    VBO objectVBO;
    VBO floorVBO;
    QOpenGLFramebufferObject* depthFBO;
    QOpenGLTexture* depthTexture;

public:
    explicit ShadowMappingWidget(QWidget* parent = NULL);
    ~ShadowMappingWidget();

protected:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int width, int height) override;

private:
    void shadowMapping(QMatrix4x4* depthMVP);
    void drawScene(const QMatrix4x4& depthMVP);

    QOpenGLShaderProgram* compileShader(const QString& vShaderFile, const QString& fShaderFile);
    void drawVBO(QOpenGLShaderProgram* const program, const VBO& vbo);
};

#endif // _SHADOW_MAPPING_WIDGET_H_
