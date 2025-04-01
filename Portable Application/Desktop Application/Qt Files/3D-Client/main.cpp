#include "homemenu.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    HomeMenu w;
    w.show();
    return a.exec();
}
