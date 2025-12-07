#include "widget.h"
#include <QApplication>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // show application folder for debugging
    qDebug() << "Application directory:" << QCoreApplication::applicationDirPath();

    Widget w;
    w.show();

    return a.exec();
}
