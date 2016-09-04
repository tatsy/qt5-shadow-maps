#ifdef _MSC_VER
#pragma once
#endif

#ifndef _OPENGL_VIEWER_H_
#define _OPENGL_VIEWER_H_

#include <memory>

#include <QtWidgets/qopenglwidget.h>
#include <QtGui/qopenglshaderprogram.h>
#include <QtGui/qopenglfunctions.h>
#include <QtGui/qopenglframebufferobject.h>
#include <QtGui/qopengltexture.h>

#include "vertexarray.h"
#include "arcballcamera.h"

enum class ShadowMapType : int {
    SM = 0x01,
    RSM = 0x02,
    ISM = 0x04
};

class OpenGLViewer : public QOpenGLWidget, protected QOpenGLFunctions {
    Q_OBJECT
    
public:
    explicit OpenGLViewer(QWidget *parent = nullptr);
    virtual ~OpenGLViewer();
    
    inline void setShadowMode(ShadowMapType type) {
        smType = type;
    }
    
protected:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int w, int h) override;
    
    void mousePressEvent(QMouseEvent *ev) override;
    void mouseMoveEvent(QMouseEvent *ev) override;
    void mouseReleaseEvent(QMouseEvent *ev) override;
    void wheelEvent(QWheelEvent *ev) override;

private:
    VertexArray *vao = nullptr;
    ArcballCamera *camera = nullptr;
    
    std::unique_ptr<QOpenGLShaderProgram> shader = nullptr;
    
    std::unique_ptr<QOpenGLShaderProgram> rsmShader = nullptr;
    std::unique_ptr<QOpenGLFramebufferObject> rsmFbo = nullptr;
    
    std::unique_ptr<QOpenGLTexture> randTexture = nullptr;
    
    ShadowMapType smType;
};

#endif  // _OPENGL_VIEWER_H_
