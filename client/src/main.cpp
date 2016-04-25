#include "ui/login.h"
#include <QApplication>
#include <QMessageBox>
#include "config.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // load config
    if (!Config::load("default.cfg")) {
        qDebug() << "NO CONFIG!";
        QMessageBox::warning(NULL, "错误", "没有找到配置文件default.cfg，请检查客户端是否完整",
                             QMessageBox::Ok, QMessageBox::Ok);
        exit(1);
    }

    Login *w = new Login();
    w->show();

    return a.exec();
}
