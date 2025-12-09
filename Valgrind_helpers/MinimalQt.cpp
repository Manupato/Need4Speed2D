#include "MinimalQt.h"

#include <QApplication>
#include <QFontDatabase>
#include <QGuiApplication>
#include <QIcon>
#include <QPixmap>
#include <QScreen>
#include <QStyleFactory>
#include <QTimer>
#include <QWidget>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    for (const QString& styleName: QStyleFactory::keys()) {
        QStyleFactory::create(styleName);
    }

    QFontDatabase::addApplicationFontFromData(QByteArray{});
    QIcon::fromTheme("folder");

    for (QScreen* s: QGuiApplication::screens()) {
        s->geometry();
    }

    QPixmap tmp;
    tmp.load(":/imagen.png");

    QWidget w;
    w.resize(400, 300);
    w.show();

    QTimer::singleShot(200, &app, &QApplication::quit);

    return app.exec();
}
