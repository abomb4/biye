#include "login.h"
#include "ui_login.h"

#include <QObject>

#include "fxclient.h"
using namespace FxChat;

Login::Login(QWidget *parent) : QDialog(parent), ui(new Ui::Login) {
    ui->setupUi(this);

    QObject::connect(ui->btn_login, SIGNAL(clicked()), this, SLOT(_do_login()));
}

Login::~Login() {
    delete ui;
}

// slot, when btn_login clicked
void Login::_do_login() {
    this->ui->lbl_errmsg->setText(QString::asprintf("00000"));
    QString name = this->ui->edt_name->text();
    QString password = this->ui->edt_password->text();

    FxChatError e = FxClient::login(&name, &password);
    if (e == FxChatError::FXM_SUCCESS) {
    } else {

    }
    this->ui->lbl_errmsg->setText(QString::asprintf("%d", e));
}
