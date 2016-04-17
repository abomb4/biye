#include "ui/login.h"
#include <QApplication>

#include "config.h"

int main(int argc, char *argv[])
{
    // load config
    if (!Config::load("default.cfg")) {
        exit(1);
    }

    QApplication a(argc, argv);

    Login w;
    w.show();

    return a.exec();
}
