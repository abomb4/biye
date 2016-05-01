#include "ui/login.h"
#include <QApplication>
#include <QMessageBox>

#include "config.h"
#include "clientdb.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QApplication::setApplicationName("fxchat");
    // QApplication::setOrganizationName("abomb4");

    // load config
    if (!Config::load("default.cfg")) {
        qDebug() << "NO CONFIG!";
        QMessageBox::warning(NULL, "错误", "没有找到配置文件default.cfg，请检查客户端是否完整",
                             QMessageBox::Ok, QMessageBox::Ok);
        exit(1);
    }
    ClientDB::getInstance()->initdb();

    Login *w = new Login();
    w->show();

    return a.exec();
}
