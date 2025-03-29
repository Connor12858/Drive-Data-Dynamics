#include "homewindow.h"

#include <QApplication>
#include <QCoreApplication>

using namespace Qt;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    HomeWindow w;
    w.setWindowFlags(FramelessWindowHint);
    w.show();

    return a.exec();
}
