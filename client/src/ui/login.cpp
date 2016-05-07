#include "login.h"
#include "ui_login.h"

#include <QObject>
#include <QRunnable>
#include <QThreadPool>

#include "fxclient.h"
using namespace FxChat;

Login::Login(QWidget *parent) : QDialog(parent), ui(new Ui::Login) {
    ui->setupUi(this);
    this->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    QObject::connect(ui->btn_login, SIGNAL(clicked()), this, SLOT(_do_login()));

    this->__logining = false;
    this->__logined = false;
}

Login::~Login() {
    delete ui;
    if (!this->__logined) {
        FxClient::quitApplication();
    }
    qDebug() << "~Login()";
}

// slot, when btn_login clicked, on main thread
void Login::_do_login() {
    if (this->__logining)
        return;
    this->__logining = true;
    this->ui->lbl_errmsg->setText(tr("正在登录…"));
    QString name = this->ui->edt_name->text();
    QString password = this->ui->edt_password->text();
    loginThread *t = new loginThread(name, password);
    connect(t, SIGNAL(finished(unsigned short)), this, SLOT(__login_finish(unsigned short)), Qt::QueuedConnection);
    QThreadPool::globalInstance()->start(t);
}

// login thread
void loginThread::run() {
    FxChatError e = FxClient::login(&this->_name, &this->_password);
    this->finished(e);
}

// slot, before login thread finishd, on main thread
void Login::__login_finish(unsigned short e) {
    if (e == FxChatError::FXM_SUCCESS) { // login success
        this->ui->lbl_errmsg->setText(tr("登录成功，初始化主界面…"));
        w = new MainWindow(); // create mainwindow
        afterLoginThread *t = new afterLoginThread(w);
        connect(t, SIGNAL(finished()), this, SLOT(__after_login_finish()), Qt::QueuedConnection);
        QThreadPool::globalInstance()->start(t);
        this->__logining = true;
        this->__logined = true;
    } else {
        switch(e) {
        case FxChatError::FXM_FAIL:
            this->ui->lbl_errmsg->setText(tr("用户名或密码错误！"));
            break;
        case FxChatError::FXM_SOCKET_ERR:
            this->ui->lbl_errmsg->setText(tr("连接服务器失败！"));
            break;
        case FxChatError::FXM_TIME_OUT:
            this->ui->lbl_errmsg->setText(tr("连接服务器超时！"));
            break;
        default:
            this->ui->lbl_errmsg->setText(tr("登录失败！"));
        }
        this->__logining = false;
    }
}

// after login thread start
void afterLoginThread::run() {
    this->_window->initData();
    this->finished();
}

// after login thread finished, on main thread
void Login::__after_login_finish() {
    w->initUi();
    w->show();
    delete this;
}
