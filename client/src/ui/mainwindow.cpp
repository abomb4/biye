#include "mainwindow.h"
#include "ui_mainwindow.h"

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
    //      2 get self info
    _user_self = FxChat::FxClient::getUserInfo();
    //      3 get recent list from db
    e = FxChat::FxClient::getRecent(this->_recent);
    //      4 get online list from server
}

void MainWindow::initUi() {
    // 0 init department tree
    // 1 place users to list
    QVector<User>::const_iterator i;
    for (i = this->_users->constBegin(); i != this->_users->constEnd(); i++) {
        ContactManager::Contact *c = ContactManager::createContact(i->id(), i->trueName());
        ContactWidget *w;

        // add to all list
        w = c->createWidget();
        QListWidgetItem *item = new QListWidgetItem(ui->list_recent);
        ui->list_recent->setItemWidget(item, w);
        w->show();

        // add to recent list

        // add to department list
        if (i->department() == this->_user_self->department()) {
            item = new QListWidgetItem(ui->list_department);
            w = c->createWidget();
            ui->list_department->setItemWidget(item, w);
        }
    }
    QTreeWidgetItem *w = new QTreeWidgetItem(this->ui->tree_all);

    connect(ui->list_department, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(openChatWindow(QListWidgetItem*)));
    connect(ui->list_recent, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(openChatWindow(QListWidgetItem*)));
    // 2 refresh online
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
    uint32_t userid = ((ContactWidget*)(item->listWidget()->itemWidget(item)))->userid();
    QString s;
    QMessageBox msgBox;
    msgBox.setWindowTitle("chat");
    msgBox.setText(s.sprintf("chat to %d？", userid));
    msgBox.setStandardButtons(QMessageBox::Close);
    msgBox.exec();
    ChatingWindow *c = new ChatingWindow(this);
    c->show();
}


// ContactManager and Contact and ContactWidget
// ContactManager::Contact
ContactManager::Contact::Contact(uint32_t uid, const QString &name) {
    this->_userid = uid;
    this->_name = name;
    this->_online = false;
    this->_widgets = new QVector<ContactWidget*>();
}

ContactWidget *ContactManager::Contact::createWidget() {
    ContactWidget *w = new ContactWidget(this);
    w->name(this->_name);
    w->userid(this->_userid);
    this->_widgets->append(w);
    return w;
}

void ContactManager::Contact::removeWidget(ContactWidget *w) {
    if (_widgets->indexOf(w) < 0) {
        qDebug() << "REMOVE WIDGET FAIL!";
    }
    _widgets->remove(_widgets->indexOf(w));
}

void ContactManager::Contact::toOnline() {
    QVector<ContactWidget*>::const_iterator i;
    for (i = this->_widgets->begin(); i != this->_widgets->end(); i++) {
        (*i)->toOnline();
    }
};
void ContactManager::Contact::toOffline() {
    QVector<ContactWidget*>::iterator i;
    for (i = this->_widgets->begin(); i != this->_widgets->end(); i++) {
        (*i)->toOffline();
    }
};

// ContactManager
QMap<uint32_t, ContactManager::Contact> ContactManager::_contact_map;

ContactManager::Contact* ContactManager::createContact(uint32_t uid, const QString &name) {
    if (!_contact_map.contains(uid)) {
        Contact c(uid, name);
        return &_contact_map.insert(uid, c).value();
    }
    return &_contact_map.find(uid).value();
}

// Widget
ContactWidget::ContactWidget(QWidget* parent, Qt::WindowFlags f) : QWidget(parent, f) {
    this->_layout = new QHBoxLayout(this);
    this->_layout->setContentsMargins(0, 0, 0, 0);

    this->_icon = new QLabel(this);
    this->_icon->setObjectName(QStringLiteral("icon"));
    this->_icon->setPixmap(QPixmap(QString::fromUtf8(":/ui/icons/im-user.svg")));
    this->_layout->addWidget(this->_icon);

    this->_name = new QLabel(this);
    this->_name->setMargin(0);
    this->_layout->addWidget(this->_name);

    this->_spacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    this->_layout->addSpacerItem(this->_spacer);
    this->setLayout(this->_layout);
}
ContactWidget::ContactWidget(ContactManager::Contact *p, QWidget* parent, Qt::WindowFlags f)
    : ContactWidget(parent, f) {
    this->_create_from = p;
}

ContactWidget::~ContactWidget() {
    this->_create_from->removeWidget(this);
    this->~QWidget();
    delete this->_icon;
    delete this->_name;
    delete this->_layout;
    delete this->_spacer;
}

void ContactWidget::toOnline() {
    this->_icon->setPixmap(QPixmap(QString::fromUtf8(":/ui/icons/im-user-online.svg")));
    this->_online = true;
}
void ContactWidget::toOffline() {
    this->_icon->setPixmap(QPixmap(QString::fromUtf8(":/ui/icons/im-user.svg")));
    this->_online = false;
}
bool ContactWidget::isOnline() { return this->_online; }

uint32_t ContactWidget::userid() const { return this->_userid; }
void ContactWidget::userid(uint32_t id) { this->_userid = id; }

void ContactWidget::name(const QString &name) { this->_name->setText(name); }
