#ifdef _MSC_VER
#pragma once
#endif

#ifndef _ARCBALL_CONTROLLER_H_
#define _ARCBALL_CONTROLLER_H_

#include <QtWidgets/qwidget.h>
#include <QtCore/qpoint.h>
#include <QtGui/qvector2d.h>
#include <QtGui/qvector3d.h>
#include <QtGui/qmatrix4x4.h>

enum class ArcballMode : int {
    None      = 0x00,
    Translate = 0x01,
    Rotate    = 0x02,
    Scale     = 0x04,
};

/** Arcball controller class.
 * @details
 * Left drag: Rotation.
 * Right drag: Translation.
 * Wheel: Scaling.
 */
class ArcballController {
public:
    // Public methods

    /** Constructor. 
     */
    explicit ArcballController(QWidget* parent = nullptr);

    /** Destructor.
     */
    ~ArcballController();

    /** Set initial modelview matrix.
     */
    void initModelView(const QMatrix4x4& modelMat, const QMatrix4x4& viewMat);

    /** Update the matrices.
     */
    void update();

    inline QMatrix4x4 modelMatrix() const { return _modelMat; }
    inline QMatrix4x4 viewMatrix() const { return _viewMat; }
    inline QMatrix4x4 modelviewMatrix() const { return _viewMat * _modelMat; }

    inline double scroll() const { return _scroll; }
    inline void setMode(ArcballMode mode) { _mode = mode; }

    inline void setOldPoint(const QPoint& pos) { this->_oldPoint = pos; }
    inline void setNewPoint(const QPoint& pos) { this->_newPoint = pos; }
    inline void setScroll(double scroll) { this->_scroll = scroll; }

private:
    // Private methods

    /** Compute the vector from arcball center to the clicked point in arcball space.
     *  @param[in] x: Click x-position.
     *  @param[in] y: Click y-position.
     */
    QVector3D getVector(int x, int y) const;

    void updateRotate();
    void updateTranslate();
    void updateScale();

    // Private fields
    QWidget* _parent;
    QPoint _newPoint;
    QPoint _oldPoint;
    QMatrix4x4 _rotMat;
    QMatrix4x4 _lookMat;
    QMatrix4x4 _modelMat;
    QMatrix4x4 _viewMat;
    QVector3D _trans;
    double _scroll;
    ArcballMode _mode;
};


#endif  // _ARCBALL_CONTROLLER_H_
