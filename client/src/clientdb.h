#ifndef CLIENTDB_H
#define CLIENTDB_H

#define DB_FILE_NAME "fxclient.dat"

#include <QtSql>

#include "structs/user.h"
#include "structs/chatlog.h"
#include "structs/department.h"

class ClientDB
{
public:
    static ClientDB *getInstance();
    ~ClientDB();
    void initdb();

    // user
    QVector<User> getUsers();
    User getUserById(uint32_t id);
    QVector<User> getUserByDepartmentId(uint32_t id);
    bool modifyUsersById(QVector<User> users);
    bool addUsers(QVector<User> users);
    bool deleteUsers(QVector<uint32_t> userids);

    // chatlog
    QVector<ChatLog> getLogs(uint32_t userid);
    uint32_t getLogSum(uint32_t userid);
    QVector<ChatLog> getLogsPage(uint32_t userid, uint32_t pagesize, uint32_t pageno);
    QVector<ChatLog> getLogsDateTime(uint32_t userid, QDateTime start, QDateTime end);
    bool addLog(const ChatLog l);

    // department
    QVector<Department> getDepartments();
    bool addDepartments(QVector<Department>);
    bool deleteDepartments(QVector<Department>);
    bool modifyDepartmentsById(QVector<Department>);

private:
    static ClientDB *_instance;
    ClientDB();
    QSqlDatabase _db;
};

#endif // CLIENTDB_H
