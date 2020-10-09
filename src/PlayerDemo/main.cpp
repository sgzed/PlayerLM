#include "PlayerDemo.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    PlayerDemo w;
    w.show();
    return a.exec();
}
