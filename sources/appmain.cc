#include <QtWidgets/qapplication.h>

#include "maingui.h"

int main(int argc, char** argv) {
    QApplication app(argc, argv);
    
    MainGUI gui;
    gui.showMaximized();

    return app.exec();
}
