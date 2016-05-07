#ifndef CLIENTDB_H
#define CLIENTDB_H

#define DB_FILE_NAME "fxclient.dat"

#include <QtSql>

#include "structs/user.h"
#include "structs/chatlog.h"
#include "structs/department.h"

namespace ClientDB
{
void initdb();
void destroydb();
// user
QVector<User> *getUsers();
User *getUserById(uint32_t id);
QVector<User> *getUserByDepartmentId(uint32_t id);
bool modifyUsersById(const QVector<User> *users);
bool addUsers(const QVector<User> *users);
bool deleteUsers(const QVector<uint32_t> *userids);

// chatlog
QVector<ChatLog> *getLogs(uint32_t userid);
uint32_t getLogSum(uint32_t userid);
QVector<ChatLog> *getLogsPage(uint32_t userid, uint32_t pagesize, uint32_t pageno);
QVector<ChatLog> *getLogsDateTime(uint32_t userid, const QDateTime start, const QDateTime end);
bool addLog(const ChatLog l);

// department
QVector<Department> getDepartments();
bool addDepartments(const QVector<Department>*);
bool deleteDepartments(const QVector<Department>*);
bool modifyDepartmentsById(const QVector<Department>*);

// status
QString getLastUserUpdateTime();
QString getLastDepartmentUpdateTime();
QString getLastUserName();
bool setLastUserUpdateTime(const QString t);
bool setLastDepartmentUpdateTime(const QString t);
bool setLastUserName(const QString name);

// recent_contact
QVector<uint32_t> *getRecent(const uint32_t userid);
bool addRecent(const uint32_t userid, const uint32_t targetid);
bool delRecent(const uint32_t userid, const uint32_t targetid);
}
#endif // CLIENTDB_H
