#include <QtWidgets/qapplication.h>
#include <QtGui/qsurfaceformat.h>

#include "maingui.h"

int main(int argc, char** argv) {
    QApplication app(argc, argv);

    QSurfaceFormat format = QSurfaceFormat::defaultFormat();
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setOption(QSurfaceFormat::DeprecatedFunctions, false);
    
    MainGUI gui;
    gui.showMaximized();

    return app.exec();
}
