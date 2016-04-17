#ifndef MYSQLCONNECTOR_H
#define MYSQLCONNECTOR_H

#define MYSQLPP_MYSQL_HEADERS_BURIED

#include <mysql++/mysql++.h>
#include <string>

#include "mysqlconnectionpool.h"

using std::string;

///
/// \brief 与mysql++库高耦合的mysql连接统一管理类
///
class MysqlConnector
{
public:
    // Get singleton class instance
    static MysqlConnector *getInstance();
    ~MysqlConnector();

    ///
    /// \brief Get connection instance, MUST RELEASE IT AFTER FINISH
    ///
    mysqlpp::Connection *getConnection();

    void release(const mysqlpp::Connection* pc);

private:
    static MysqlConnector *s_instance;

    MysqlConnector();

    // mysql connection pool
    MysqlConnectionPool *pool;

    string user, password, host, db;
    unsigned int port;
};

#endif // MYSQLCONNECTOR_H
