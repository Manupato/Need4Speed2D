#include <QApplication>

#include "../common/resource_paths.h"

#include "mainwindow.h"

int main(int argc, char* argv[]) {
    ResourcePaths::init();
    QApplication app(argc, argv);

    MainWindow window;
    window.show();

    return app.exec();
}
