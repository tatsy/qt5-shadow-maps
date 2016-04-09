#include <QtWidgets/qapplication.h>

#include "shadowmaps_widget.h"

int main(int argc, char** argv) {
    QApplication app(argc, argv);

    ShadowMapsWidget widget;
    widget.resize(800, 600);
    widget.show();

    return app.exec();
}
