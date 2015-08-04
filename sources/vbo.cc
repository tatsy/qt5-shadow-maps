#include "vbo.h"

#include <iostream>
#include <fstream>
#include <sstream>

VBO::VBO()
    : _vertices()
    , _normals()
    , _colors()
    , _indices()
{
}

VBO::VBO(const VBO& vbo)
    : _vertices()
    , _normals()
    , _colors()
    , _indices()
{
    this->operator=(vbo);
}

VBO::VBO(VBO&& vbo)
    : _vertices()
    , _normals()
    , _colors()
    , _indices()
{
    this->operator=(std::move(vbo));
}

VBO::~VBO()
{
}

VBO& VBO::operator=(const VBO& vbo) {
    this->_vertices = vbo._vertices;
    this->_normals  = vbo._normals;
    this->_colors   = vbo._colors;
    this->_indices  = vbo._indices;
    return *this;
}

VBO& VBO::operator=(VBO&& vbo) {
    this->_vertices = std::move(vbo._vertices);
    this->_normals  = std::move(vbo._normals);
    this->_colors   = std::move(vbo._colors);
    this->_indices  = std::move(vbo._indices);
    return *this;
}

VBO VBO::fromObjFile(const char* filename, const QVector3D color) {
    std::ifstream ifs(filename, std::ios::in);
    if (!ifs.is_open()) {
        fprintf(stderr, "[ERROR] failed to open file \"%s\"\n", filename);
        std::abort();
    }

    std::stringstream ss;
    std::string line, typ;

    VBO vbo;
    while(!ifs.eof()) {
        std::getline(ifs, line);

        // Check first character
        auto it = line.begin();
        while (it != line.end() && *it == ' ') ++it;
        if (it == line.end() || *it == '#') continue;

        // Read line
        ss.clear();
        ss << line;
        ss >> typ;

        if (typ == "v") {
            float x, y, z;
            ss >> x >> y >> z;
            vbo._vertices.push_back(QVector3D(x, y, z));
        } else if (typ == "f") {
            int i, j, k;
            ss >> i >> j >> k;
            Triplet triplet = { i - 1, j - 1, k - 1 };
            vbo._indices.push_back(triplet);
        } else {
            fprintf(stderr, "Unknown type \"%s\" is found while reading .obj file!!", typ.c_str());
            std::abort();
        }
    }

    const int numVerts = (int)vbo._vertices.size();
    vbo._normals.assign(numVerts, QVector3D(0.0f, 0.0f, 0.0f));
    std::vector<int> faceCount(numVerts, 0);

    for (int i = 0; i < vbo._indices.size(); i++) {
        const QVector3D v0 = vbo._vertices[vbo._indices[i].i];
        const QVector3D v1 = vbo._vertices[vbo._indices[i].j];
        const QVector3D v2 = vbo._vertices[vbo._indices[i].k];
        
        const QVector3D fNormal = QVector3D::crossProduct(v1 - v0, v2 - v0).normalized();
        vbo._normals[vbo._indices[i].i] += fNormal;
        vbo._normals[vbo._indices[i].j] += fNormal;
        vbo._normals[vbo._indices[i].k] += fNormal;
        faceCount[vbo._indices[i].i] += 1;
        faceCount[vbo._indices[i].j] += 1;
        faceCount[vbo._indices[i].k] += 1;
    }

    for (int i = 0; i < numVerts; i++) {
        vbo._normals[i] /= faceCount[i];
    }

    vbo._colors.assign(numVerts, color);

    return std::move(vbo);
}

VBO VBO::checkerFloor(QVector3D normal, double dist, double size, int rows, int cols, const QVector3D color1, const QVector3D color2) {
    QVector3D u, v, w;
    w = normal;
    if (normal.x() > 1.0e-6) {
        v = QVector3D::crossProduct(QVector3D(0.0f, 1.0f, 0.0f), w);
    } else {
        v = QVector3D::crossProduct(QVector3D(1.0f, 0.0f, 0.0f), w);
    }
    u = QVector3D::crossProduct(w, v);

    VBO vbo;

    QVector3D origin = - dist * w;
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            QVector3D v0 = origin - (i - rows / 2) * v * size - (j - cols / 2) * u * size;
            QVector3D v1 = v0 + u * size;
            QVector3D v2 = v0 + v * size;
            QVector3D v3 = v0 + u * size + v * size;

            const int vid = vbo._vertices.size();

            vbo._vertices.push_back(v0);
            vbo._vertices.push_back(v1);
            vbo._vertices.push_back(v2);
            vbo._vertices.push_back(v3);

            vbo._normals.push_back(normal);
            vbo._normals.push_back(normal);
            vbo._normals.push_back(normal);
            vbo._normals.push_back(normal);

            QVector3D color = (i + j) % 2 == 0 ? color1 : color2;
            vbo._colors.push_back(color);
            vbo._colors.push_back(color);
            vbo._colors.push_back(color);
            vbo._colors.push_back(color);

            Triplet f1 = { vid, vid + 3, vid + 1 };
            vbo._indices.push_back(f1);
            Triplet f2 = { vid, vid + 2, vid + 3 };
            vbo._indices.push_back(f2);
        }
    }

    return std::move(vbo);
}