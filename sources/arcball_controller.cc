#include "arcball_controller.h"

#include <cmath>

ArcballController::ArcballController(QWidget* parent)
    : _parent{parent}
    , _newPoint{0, 0}
    , _oldPoint{0, 0}
    , _rotMat{}
    , _lookMat{}
    , _modelMat{}
    , _viewMat{}
    , _trans{0.0, 0.0, 0.0}
    , _scroll{0.0}
    , _mode{ArcballMode::None} {
}

ArcballController::~ArcballController() {
}

void ArcballController::initModelView(const QMatrix4x4& modelMat,
                                        const QMatrix4x4& viewMat) {
    _rotMat = modelMat;
    _lookMat  = viewMat;
    update();
}

void ArcballController::update() {
    switch (_mode) {
    case ArcballMode::Translate:
        updateTranslate();
        break;

    case ArcballMode::Rotate:
        updateRotate();
        break;

    case ArcballMode::Scale:
        updateScale();
        break;

    default:
        break;
    }
    _modelMat = _rotMat;
    _modelMat.scale(1.0 - _scroll * 0.1);

    _viewMat = _lookMat;
    _viewMat.translate(_trans);
}

void ArcballController::updateTranslate() {
    const double dx = 10.0 * (_newPoint.x() - _oldPoint.x()) / _parent->width();
    const double dy = 10.0 * (_newPoint.y() - _oldPoint.y()) / _parent->height();
    _trans += QVector3D(dx, -dy, 0.0);
}

void ArcballController::updateRotate() {
    const QVector3D u = getVector(_newPoint.x(), _newPoint.y());
    const QVector3D v = getVector(_oldPoint.x(), _oldPoint.y());

    const double angle = acos(std::min(1.0f, QVector3D::dotProduct(u, v)));

    const QVector3D rotAxis = QVector3D::crossProduct(v, u);
    const QMatrix4x4 camera2objMat = _rotMat.inverted();

    const QVector3D objSpaceRotAxis = camera2objMat * rotAxis;

    QMatrix4x4 temp;
    double angleByDegree = 180.0 * angle / M_PI;
    temp.rotate(4.0 * angleByDegree, objSpaceRotAxis);

    _rotMat = _rotMat * temp;     
}

void ArcballController::updateScale() {
    const double dy = 20.0 * (_newPoint.y() - _oldPoint.y()) / _parent->height();
    _scroll += dy;
}

QVector3D ArcballController::getVector(int x, int y) const {
    QVector3D pt( 2.0 * x / _parent->width()  - 1.0,
                    -2.0 * y / _parent->height() + 1.0,
                    0.0);

    const double xySquared = pt.x() * pt.x() + pt.y() * pt.y();

    if (xySquared <= 1.0) {
        pt.setZ(sqrt(1.0 - xySquared));
    } else {
        pt.normalize();
    }

    return pt;
}
