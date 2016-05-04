#include "clientdb.h"

#include <QMessageBox>
#include <QStandardPaths>

ClientDB *ClientDB::_instance = nullptr;

ClientDB *ClientDB::getInstance() {
    if (_instance == nullptr) {
        _instance = new ClientDB();
    }
    return _instance;
}

ClientDB::ClientDB() {}

bool createDB(const QString &path) {
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(path);
    if (!db.open()) {
        return false;
    }
    bool r;
    QSqlQuery query;
    r = query.exec("create table `fx_user` ("
        "`id` integer primary key,"
        "`name` varchar(32) not null unique,"
        "`email` varchar(32),"
        "`true_name` varchar(16),"
        "`department` integer not null,"
        "`icon` text,"
        "`status` int(1) not null default 1,"
        "`gmt_create` datetime not null,"
        "`gmt_modify` datetime not null"
        ");");if (!r)return r;
    r = query.exec("create table `fx_department` ("
        "`id` integer primary key,"
        "`parent_id` integer not null default 0,"
        "`name` varchar(32) not null,"
        "`gmt_create` datetime not null,"
        "`gmt_modify` datetime not null"
        ");");if (!r)return r;
    r = query.exec("create table `fx_chatlog` ("
        "`id` integer primary key,"
        "`sourceid` integer not null,"
        "`targetid` integer not null,"
        "`type` int(1) not null,"
        "`msg` text,"
        "`gmt_create` datetime not null"
        ");");if (!r)return r;
    r = query.exec("create table `fx_recent_contact` ("
        "`userid` integer not null,"
        "`contactid` integer not null,"
        "`last_time` datetime not null,"
        "constraint pk_recent primary key (userid, contactid)"
        ");");if (!r)return r;

    r = query.exec("create table `status` ("
        "`key` varchar(64) not null primary key,"
        "`value` varchar(255) not null default ''"
        ");");if (!r) return r;

    r = query.exec("insert into `status` values"
        "('last_user_update_time', ''),"
        "('last_department_update_time', ''),"
        "('last_user_name', '');");if (!r) return r;

    db.close();
    return true;
}

void ClientDB::initdb() {
    QString dbpath = QStandardPaths::locate(QStandardPaths::AppDataLocation, DB_FILE_NAME);
    if (dbpath.isEmpty()) { // create
        QDir dir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
        if (!dir.exists()){
            if (!dir.mkpath(".")) {
                QMessageBox::critical(0, "Cannot create dir",
                       "Cannot create" + dir.path(), QMessageBox::Cancel);
                exit(2);
            }
        }
        dbpath = dir.absoluteFilePath(DB_FILE_NAME);
        if (!createDB(dbpath)) {
            QMessageBox::critical(0, "Cannot open database",
                   "Create SQLite database fail.", QMessageBox::Cancel);
            exit(1);
            return;
        }
    }
    this->_db = QSqlDatabase::addDatabase("QSQLITE");
    this->_db.setDatabaseName(dbpath);
    if (!this->_db.open()) {
        QMessageBox::critical(0, "Cannot open database",
               "Open SQLite database fail.", QMessageBox::Cancel);
        exit(1);
        return;
    }
}

///////////////////////// user /////////////////////////
QVector<User> *ClientDB::getUsers() {
    QSqlQuery query;
    query.exec("select id, name, email, true_name, department, icon, gmt_create, gmt_modify"
               "from fx_user where status = 1");
    QVector<User> *v = new QVector<User>();
    do {
        User u;
        u.id(query.value(0).toInt());
        u.name(query.value(1).toString());
        u.email(query.value(2).toString());
        u.trueName(query.value(3).toString());
        u.department(query.value(4).toInt());
        u.icon(query.value(5).toString());
        u.status(1);
        u.gmtCreate(query.value(6).toDateTime());
        u.gmtModify(query.value(7).toDateTime());
        v->append(u);
    } while(query.next());
    return v;
}
User *ClientDB::getUserById(uint32_t id) {
    QSqlQuery query;
    query.prepare("select id, name, email, true_name, department, icon, gmt_create, gmt_modify, status"
               "from fx_user where id = ?");
    query.bindValue(0, id);
    if (!query.exec()) {
        qDebug() << query.lastError();
        return nullptr;
    };
    User *u = new User();
    u->id(query.value(0).toInt());
    u->name(query.value(1).toString());
    u->email(query.value(2).toString());
    u->trueName(query.value(3).toString());
    u->department(query.value(4).toInt());
    u->icon(query.value(5).toString());
    u->gmtCreate(query.value(6).toDateTime());
    u->gmtModify(query.value(7).toDateTime());
    u->status(query.value(8).toInt());
    return u;
}
QVector<User> *ClientDB::getUserByDepartmentId(uint32_t id) {}
bool ClientDB::modifyUsersById(const QVector<User> *users) {}
bool ClientDB::addUsers(const QVector<User> *users) {
    QSqlQuery q;
        q.prepare("insert into fx_user(id, name, true_name, department, gmt_create, gmt_modify, status)"
                  "values (?, ?, ?, ?, date(), date(), 1)");

    QVariantList ids, names, true_names, departments;
    QVector<User>::const_iterator i = users->begin();
    for (QVector<User>::const_iterator i = users->begin(); i != users->end(); i++) {
        ids << i->id();
        names << i->name();
        true_names << i->trueName();
        departments << i->department();
    }
    q.addBindValue(ids);
    q.addBindValue(names);
    q.addBindValue(true_names);
    q.addBindValue(departments);
    bool r;
    if (!(r = q.execBatch()))
        qDebug() << "ClientDB addUsers FAIL!" << q.lastError();
    return r;
}
bool ClientDB::deleteUsers(const QVector<uint32_t> *userids) {}

///////////////////////// chatlog /////////////////////////
QVector<ChatLog> *ClientDB::getLogs(uint32_t userid) {}
uint32_t ClientDB::getLogSum(uint32_t userid) {}
QVector<ChatLog> *ClientDB::getLogsPage(uint32_t userid, uint32_t pagesize, uint32_t pageno) {}
QVector<ChatLog> *ClientDB::getLogsDateTime(uint32_t userid, const QDateTime start, const QDateTime end) {}
bool ClientDB::addLog(const ChatLog l) {}

///////////////////////// department /////////////////////////
QVector<Department> ClientDB::getDepartments() {}
bool ClientDB::addDepartments(const QVector<Department>*) {}
bool ClientDB::deleteDepartments(const QVector<Department>*) {}
bool ClientDB::modifyDepartmentsById(const QVector<Department>*) {}

///////////////////////// status /////////////////////////
QString ClientDB::getLastUserUpdateTime() const {
    QSqlQuery query;
    query.exec("select value from `status` where `key` = 'last_user_update_time'");
    return query.value(0).toString();
}
QString ClientDB::getLastDepartmentUpdateTime() const {
    QSqlQuery query;
    query.exec("select value from `status` where `key` = 'last_department_update_time'");
    return query.value(0).toString();
}
QString ClientDB::getLastUserName() const {
    QSqlQuery query;
    query.exec("select value from `status` where `key` = 'last_user_name'");
    return query.value(0).toString();
}
bool ClientDB::setLastUserUpdateTime(const QString t) {
    QSqlQuery query;
    query.prepare("update `status` set `value` = ? where `key` = 'last_user_update_time'");
    query.bindValue(0, t);
    return query.exec();
}
bool ClientDB::setLastDepartmentUpdateTime(const QString t) {
    QSqlQuery query;
    query.prepare("update `status` set `value` = ? where `key` = 'last_department_update_time'");
    query.bindValue(0, t);
    return query.exec();
}
bool ClientDB::setLastUserName(const QString t) {
    QSqlQuery query;
    query.prepare("update `status` set `value` = ? where `key` = 'last_user_name'");
    query.bindValue(0, t);
    return query.exec();
}

///////////////////////// recent_contact /////////////////////////
QVector<uint32_t> *ClientDB::getRecent(const uint32_t userid) {}
bool ClientDB::addRecent(const uint32_t userid, const uint32_t targetid) {}
bool ClientDB::delRecent(const uint32_t userid, const uint32_t targetid) {}
