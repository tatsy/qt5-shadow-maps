#ifdef _MSC_VER
#pragma once
#endif

#ifndef _VERTEX_ARRAY_H_
#define _VERTEX_ARRAY_H_

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include <QtGui/qopenglshaderprogram.h>
#include <QtGui/qopenglvertexarrayobject.h>
#include <QtGui/qopenglbuffer.h>
#include <QtGui/qopenglcontext.h>
#include <QtGui/qopenglextrafunctions.h>
#include <QtGui/qvector3d.h>

#include "tinyply.h"

class VertexArray {
public:
    explicit VertexArray() {
    }

    virtual ~VertexArray() {
        if (vbo) {
            delete vbo;
            vbo = nullptr;
        }
        
        if (ibo) {
            delete ibo;
            ibo = nullptr;
        }
        
        if (vao) {
            delete vao;
            vao = nullptr;
        }
    }

    void load(const std::string &filename) {
        // Load input file.
        std::ifstream ifs(filename.c_str(), std::ios::in | std::ios::binary);
        if (!ifs.is_open()) {
            std::cerr << "[ ERROR ] Failed to open file: " << filename << std::endl;
            return;
        }

        tinyply::PlyFile file(ifs);
        
        positions_.clear();
        normals_.clear();
        colors_.clear();
        indices_.clear();
        std::vector<uint8_t> colorBytes;

        file.request_properties_from_element("vertex", {"x", "y", "z"}, positions_);
        file.request_properties_from_element("vertex", {"nx", "ny", "nz"}, normals_);
        file.request_properties_from_element("vertex", {"red", "green", "blue", "alpha"}, colorBytes);
        file.request_properties_from_element("face", {"vertex_indices"}, indices_);
        
        file.read(ifs);
        ifs.close();
        
        colors_.resize(colorBytes.size());
        for (int i = 0; i < colorBytes.size(); i++) {
            colors_[i] = colorBytes[i] / 255.0f;
        }
        
        // Prepare VAO
        vao = new QOpenGLVertexArrayObject();
        vao->create();
        vao->bind();
        
        vbo = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
        vbo->create();
        vbo->setUsagePattern(QOpenGLBuffer::StaticDraw);
        vbo->bind();
        
        const size_t totalSize =
            (positions_.size() + normals_.size()) * sizeof(float) * 3;
        vbo->allocate(totalSize);
        
        unsigned long offset = 0;
        vbo->write(offset, &positions_[0], positions_.size() * sizeof(float));
        glEnableVertexAttribArray(POSITION_LOCATION);
        glVertexAttribPointer(POSITION_LOCATION, 3, GL_FLOAT, GL_FALSE, 0, (void*)offset);

        offset += positions_.size() * sizeof(float);
        vbo->write(offset, &normals_[0], normals_.size() * sizeof(float));
        glEnableVertexAttribArray(NORMAL_LOCATION);
        glVertexAttribPointer(NORMAL_LOCATION, 3, GL_FLOAT, GL_FALSE, 0, (void*)offset);
        
        offset += normals_.size() * sizeof(float);
        vbo->write(offset, &colors_[0], colors_.size() * sizeof(float));
        glEnableVertexAttribArray(COLOR_LOCATION);
        glVertexAttribPointer(COLOR_LOCATION, 4, GL_FLOAT, GL_FALSE, 0, (void*)offset);
        
        ibo = new QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);
        ibo->create();
        ibo->setUsagePattern(QOpenGLBuffer::StaticDraw);
        ibo->bind();
        ibo->allocate(&indices_[0], sizeof(unsigned int) * indices_.size());
        
        vao->release();
    }

    void draw(QOpenGLShaderProgram& shader) const {
        vao->bind();

        glDrawElements(GL_TRIANGLES, indices_.size(), GL_UNSIGNED_INT, 0);

        vao->release();
    }

private:
    QOpenGLVertexArrayObject *vao = nullptr;
    QOpenGLBuffer *vbo = nullptr;
    QOpenGLBuffer *ibo = nullptr;
    
    std::vector<float> positions_;
    std::vector<float> normals_;
    std::vector<float> colors_;
    std::vector<unsigned int> indices_;

    static constexpr int POSITION_LOCATION = 0;
    static constexpr int NORMAL_LOCATION   = 1;
    static constexpr int COLOR_LOCATION    = 2;
};

#endif  // _VERTEX_ARRAY_H_
