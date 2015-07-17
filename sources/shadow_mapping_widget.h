#ifndef _SHADOW_MAPPING_WIDGET_H_
#define _SHADOW_MAPPING_WIDGET_H_

#include <QtWidgets/qwidget.h>
#include <QtWidgets/qopenglwidget.h>

#include <QtGui/qopenglshaderprogram.h>

class ShadowMappingWidget : public QOpenGLWidget {
    Q_OBJECT

private:
    static const int SHADOW_MAP_SIZE = 1024;
    QOpenGLShaderProgram* renderShader;
    QOpenGLShaderProgram* shadowmapShader;

public:
    explicit ShadowMappingWidget(QWidget* parent = NULL);
    ~ShadowMappingWidget();

protected:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int width, int height) override;

private:
    static QOpenGLShaderProgram* compileShader(const QString& vShaderFile, const QString& fShaderFile);
};

#endif // _SHADOW_MAPPING_WIDGET_H_
