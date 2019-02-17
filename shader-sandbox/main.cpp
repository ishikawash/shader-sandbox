#include <QApplication>
#include <QSurfaceFormat>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QSurfaceFormat format;
    format.setProfile(QSurfaceFormat::CoreProfile);
    // format.setVersion(4, 1);
    QSurfaceFormat::setDefaultFormat(format);

    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
