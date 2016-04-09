#ifndef _VBO_H_
#define _VBO_H_

#include <vector>

#include <QtGui/qvector3d.h>

#if defined(_WIN32) || defined(__WIN32__)
#define PACKED(__declare__) __pragma(pack(push,1)) __declare__ __pragma(pack(pop))
#else
#define PACKED(__declare__) __declare__ __attribute__ ((__packed__))
#endif

PACKED(
struct Triplet {
    unsigned int i;
    unsigned int j;
    unsigned int k;
});

class VBO {
private:
    std::vector<QVector3D> _vertices;
    std::vector<QVector3D> _normals;
    std::vector<QVector3D> _colors;
    std::vector<Triplet> _indices;

public:
    VBO();
    VBO(const VBO& vbo);
    VBO(VBO&& vbo);
    ~VBO();

    VBO& operator=(const VBO& vbo);
    VBO& operator=(VBO&& vbo);

    static VBO fromObjFile(const char* filename, const QVector3D color = QVector3D(1.0f, 1.0f, 1.0f));
    static VBO colorBox();
    static VBO checkerFloor(QVector3D normal, double dist, double size, int rows, int cols, const QVector3D color1 = QVector3D(0.8f, 0.8f, 0.8f), const QVector3D color2 = QVector3D(0.5f, 0.5f, 0.5f));

    inline const std::vector<QVector3D>& vertices() const { return _vertices; }
    inline const std::vector<QVector3D>& normals()  const { return _normals; }
    inline const std::vector<QVector3D>& colors()   const { return _colors; }
    inline const std::vector<Triplet>&   indices()  const { return _indices; }
};

#endif  // _VBO_H_
