#ifndef _SHADOW_MAPPING_WIDGET_H_
#define _SHADOW_MAPPING_WIDGET_H_

#include <QtWidgets/qwidget.h>
#include <QtWidgets/qopenglwidget.h>

class ShadowMappingWidget : public QOpenGLWidget {
public:
    explicit ShadowMappingWidget(QWidget* parent = NULL);
    ~ShadowMappingWidget();

protected:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int width, int height) override;
};

#endif // _SHADOW_MAPPING_WIDGET_H_
