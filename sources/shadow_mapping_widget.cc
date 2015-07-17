#include <cstdlib>
#include <iostream>

#include "shadow_mapping_widget.h"

ShadowMappingWidget::ShadowMappingWidget(QWidget* parent)
    : QOpenGLWidget(parent)
    , renderShader(NULL)
    , shadowmapShader(NULL)
{
}

ShadowMappingWidget::~ShadowMappingWidget()
{
    delete renderShader;
    delete shadowmapShader;
}

void ShadowMappingWidget::initializeGL() {

}

void ShadowMappingWidget::paintGL() {
}

void ShadowMappingWidget::resizeGL(int width, int height) {
    glViewport(0, 0, width, height);
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

