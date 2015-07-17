#include <QtWidgets/qapplication.h>

#include "shadow_mapping_widget.h"

int main(int argc, char** argv) {
    QApplication app(argc, argv);

    ShadowMappingWidget widget;
    widget.show();

    return app.exec();
}
