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
        "`id` int primary key,"
        "`name` varchar(32) not null unique,"
        "`email` varchar(32),"
        "`password` varchar(32) not null,"
        "`true_name` varchar(16),"
        "`department` int not null,"
        "`icon` text,"
        "`status` int(1) not null default 1,"
        "`gmt_create` datetime not null,"
        "`gmt_modify` datetime not null"
        ");");
    query.exec("create table `fx_department` ("
        "`id` int primary key,"
        "`parent_id` int not null default 0,"
        "`name` varchar(32) not null,"
        "`gmt_create` datetime not null,"
        "`gmt_modify` datetime not null"
        ");");
    query.exec("create table `fx_chatlog` ("
        "`id` int primary key,"
        "`sourceid` int not null,"
        "`targetid` int not null,"
        "`type` int(1) not null,"
        "`msg` text,"
        "`gmt_create` datetime not null"
        ");");

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
QVector<User> getUsers() {}
User getUserById(uint32_t id) {}
QVector<User> getUserByDepartmentId(uint32_t id) {}
bool modifyUsersById(QVector<User> users) {}
bool addUsers(QVector<User> users) {}
bool deleteUsers(QVector<uint32_t> userids) {}

///////////////////////// chatlog /////////////////////////
QVector<ChatLog> getLogs(uint32_t userid) {}
uint32_t getLogSum(uint32_t userid) {}
QVector<ChatLog> getLogsPage(uint32_t userid, uint32_t pagesize, uint32_t pageno) {}
QVector<ChatLog> getLogsDateTime(uint32_t userid, QDateTime start, QDateTime end) {}
bool addLog(const ChatLog l) {}

///////////////////////// department /////////////////////////
QVector<Department> getDepartments() {}
bool addDepartments(QVector<Department>) {}
bool deleteDepartments(QVector<Department>) {}
bool modifyDepartmentsById(QVector<Department>) {}
