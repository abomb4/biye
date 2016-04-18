#ifndef DIALOG_H
#define DIALOG_H

#include <QRunnable>
#include <QDialog>

#include "mainwindow.h"

#include "fxclient.h"

using namespace FxChat;

namespace Ui {
class Login;
}

class Login : public QDialog
{
    Q_OBJECT

public slots:
    void _do_login();

public:
    explicit Login(QWidget *parent = 0);
    ~Login();


private slots:
    // 登录结束槽
    void __login_finish(unsigned short e);
    // 主界面数据初始化结束槽
    void __after_login_finish();

private:
    Ui::Login *ui;
    bool __logining;
    MainWindow *w;
};

///
/// \brief 登录线程
///
class loginThread : public QObject, public QRunnable {
    Q_OBJECT
public:
    loginThread(const QString name, const QString password):
        _name(name),
        _password(password)
    {
        this->setAutoDelete(true);
    }
    void run();

signals:
    void finished(unsigned short);

private:
    QString _name, _password;
};

///
/// \brief 初始化主界面数据线程
///
class afterLoginThread : public QObject, public QRunnable {
    Q_OBJECT
public:
    afterLoginThread(MainWindow *w)
    {
        this->_window = w;
        this->setAutoDelete(true);
    }
    void run();

signals:
    void finished();

private:
    MainWindow *_window;

};

#endif // DIALOG_H
