#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QtWidgets/QLabel>
#include <QMessageBox>

#include "clientdb.h"
#include "chatingwindow.h"
#include "fxclient.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->action_exit, SIGNAL(triggered(bool)), this, SLOT(askClostApp()));
    connect(ui->action_about_qt, SIGNAL(triggered(bool)), this, SLOT(aboutQt()));
}

MainWindow::~MainWindow() {
    delete ui;
}

//
void MainWindow::initData() {
    // TODO 1 get all user list from db
    FxChatError e;
    e = FxChat::FxClient::getUserList(this->_users);
    if (e != FXM_SUCCESS) {
        qDebug() << "GetUserList FAIL!!" << e;
    }
    //      3 get recent list from db
    e = FxChat::FxClient::getRecent(this->_recent);
    //      4 get online list from server
}

void MainWindow::initUi() {
    for (int i = 0; i < 10; i++) {
        QWidget *w = new QWidget();
        QHBoxLayout *layout = new QHBoxLayout(w);
        layout->setContentsMargins(0, 0, 0, 0);

        QLabel *icon = new QLabel();
        icon->setObjectName(QStringLiteral("icon"));
        icon->setPixmap(QPixmap(QString::fromUtf8(":/ui/icons/im-user.svg")));
        layout->addWidget(icon);

        QLabel *name = new QLabel();
        name->setText(tr("ojiasdfiojasdfjio"));
        name->setMargin(0);
        layout->addWidget(name);

        QSpacerItem *spacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
        layout->addSpacerItem(spacer);

        w->setLayout(layout);
        w->show();
        QListWidgetItem *item = new ContactItem(ui->listWidget);
        ui->listWidget->setItemWidget(item, w);
        item->setData(ContactItem::UserId, i);
    }
    connect(ui->listWidget, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(openChatWindow(QListWidgetItem*)));
}


// slot
void MainWindow::closeApp() {
    this->close();
}

void MainWindow::askClostApp() {
    QMessageBox msgBox;
    msgBox.setWindowTitle("退出确认");
    msgBox.setText("确认退出程序？");
    msgBox.setStandardButtons(QMessageBox::Yes| QMessageBox::No);
    switch (msgBox.exec()) {
    case QMessageBox::Yes:
        this->closeApp();
        break;
    case QMessageBox::No:
    default:
        break;
    }
}
void MainWindow::aboutQt() {
    QMessageBox::aboutQt(NULL,"AboutQt");
}

void MainWindow::openChatWindow(QListWidgetItem *item) {
    uint32_t userid = item->data(ContactItem::UserId).toUInt();
    QString s;
    QMessageBox msgBox;
    msgBox.setWindowTitle("chat");
    msgBox.setText(s.sprintf("chat to %d？", userid));
    msgBox.setStandardButtons(QMessageBox::Close);
    msgBox.exec();
    ChatingWindow *c = new ChatingWindow(this);
    c->show();
}


// ContactItem
QVariant ContactItem::data(int role) const {
    switch(role) {
    case ContactItem::UserId:
        return this->userid;
    default:
        return QVariant();
    }
}

void ContactItem::setData(int role, const QVariant & value) {
    switch(role) {
    case ContactItem::UserId:
        this->userid = value.toUInt();break;
    }
}
