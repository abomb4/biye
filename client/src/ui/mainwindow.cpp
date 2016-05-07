#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>

#include "clientdb.h"
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
    FxClient::quitApplication();
}

//
void MainWindow::initData() {
    FxClient *c = FxClient::getInstance();
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

    //      5 connect server side positive message slot
    connect(c, SIGNAL(receiveMsg(quint32,quint32,QString)),
            this, SLOT(receiveMsg(quint32,quint32,QString)),
            Qt::QueuedConnection);

    connect(c, SIGNAL(online(quint32)),
            this, SLOT(toOnline(quint32)),
            Qt::QueuedConnection);

    connect(c, SIGNAL(offline(quint32)),
            this, SLOT(toOffline(quint32)),
            Qt::QueuedConnection);

    FxClient::startListenMsg();
}

void MainWindow::initUi() {
    // 0 init department tree
    // 1 place users to list
    QMap<uint32_t, User>::const_iterator i;
    for (i = this->_users->constBegin(); i != this->_users->constEnd(); i++) {
        ContactManager::Contact *c = ContactManager::createContact(i->id(), i->trueName());
        ContactWidget *w;

        // self
        if (i->id() == this->_user_self->id()) {
            c->toOnline();
            w = c->createWidget();
            ui->c_self->layout()->addWidget(w);
        }
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
    delete this;
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
    if (_chatwindows.contains(userid)) {
        _chatwindows.value(userid)->show();
        return;
    }
    ChatingWindow *c = new ChatingWindow();
    _chatwindows.insert(userid, c);
    c->touserid(userid);
    c->show();
}
void MainWindow::chatWindowCloses(quint32 uid) {
    _chatwindows.remove(uid);
}

void MainWindow::receiveMsg(quint32 from_user_id, quint32 to_user_id, const QString &msgbody) {
    qDebug() << "MainWIndow Recieved";
    if (to_user_id != this->_user_self->id()) {
        qDebug() << "THIS IS A SERVER FAULT! SERVER SEND ANOTHER USER's MSG TO THIS CLINET!";
        return;
    }

    ChatingWindow *c = _chatwindows.value(from_user_id, nullptr);
    if (c == nullptr) {
        c = new ChatingWindow();
        _chatwindows.insert(from_user_id, c);
        c->touserid(from_user_id);
        c->appendMsg(_users->value(from_user_id).trueName(), msgbody, QDateTime::currentDateTime());
        c->show();
    } else {
        c->appendMsg(_users->value(from_user_id).trueName(), msgbody, QDateTime::currentDateTime());
    }
}

void MainWindow::toOnline(quint32 uid) {
    ContactManager::getContact(uid)->toOnline();
}
void MainWindow::toOffline(quint32 uid) {
    ContactManager::getContact(uid)->toOffline();
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
    if (this->_online)
        w->toOnline();
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
    this->_online = true;
    QVector<ContactWidget*>::const_iterator i;
    for (i = this->_widgets->begin(); i != this->_widgets->end(); i++) {
        (*i)->toOnline();
    }
};
void ContactManager::Contact::toOffline() {
    this->_online = false;
    QVector<ContactWidget*>::iterator i;
    for (i = this->_widgets->begin(); i != this->_widgets->end(); i++) {
        (*i)->toOffline();
    }
};

// ContactManager
QMap<uint32_t, ContactManager::Contact*> ContactManager::_contact_map;

ContactManager::Contact* ContactManager::createContact(uint32_t uid, const QString &name) {
    if (!_contact_map.contains(uid)) {
        Contact *c = new Contact(uid, name);
        return _contact_map.insert(uid, c).value();
    }
    return _contact_map.find(uid).value();
}
ContactManager::Contact* ContactManager::getContact(uint32_t uid) {
    return _contact_map.value(uid);
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
