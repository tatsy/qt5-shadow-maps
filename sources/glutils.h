#ifdef _MSC_VER
#pragma once
#endif

#ifndef _GL_UTILS_H_
#define _GL_UTILS_H_

#include <iostream>
#include <string>
#include <memory>

#include <QtGui/qopenglshaderprogram.h>

inline std::unique_ptr<QOpenGLShaderProgram>
    compileShader(const std::string& name, bool useGeom = false) {

    auto shader = std::make_unique<QOpenGLShaderProgram>();
    const QString basename(name.c_str());

    shader->addShaderFromSourceFile(QOpenGLShader::Vertex,
                                    basename + ".vs");
    if (useGeom) {
        shader->addShaderFromSourceFile(QOpenGLShader::Geometry,
                                        basename + ".gs");
    }
    shader->addShaderFromSourceFile(QOpenGLShader::Fragment,
                                    basename + ".fs");

    shader->link();
    if (!shader->isLinked()) {
        std::cerr << "[ERROR] failed to compile or link shader: " << name << std::endl;
        return nullptr;
    }

    return std::move(shader);    
}

#endif  // _GL_UTILS_H_
