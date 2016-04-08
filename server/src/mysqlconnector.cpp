#include "mysqlconnector.h"

#include <cstdlib>
#include <cstring>
#include <iostream>

#include <mysql++/mysql++.h>

#include "config.h"

using std::cout;
using std::endl;
using std::cerr;

MysqlConnector MysqlConnector::s_instance;

MysqlConnector &MysqlConnector::getInstance() {
    return s_instance;
}

MysqlConnector::~MysqlConnector() {
    delete this->pool;
}

MysqlConnector::MysqlConnector() {
    this->user = Config::get("mysql.user");
    this->db = Config::get("mysql.db");
    this->password = Config::get("mysql.password");
    this->host = Config::get("mysql.host");
    this->port = atoi(Config::get("mysql.port").c_str());
    if (this->port == 0)
        this->port = 3306;

    int maxc = atoi(Config::get("mysql.pool.maxconnections").c_str());
    if (maxc <= 0)
        maxc = 8;
    else if (maxc > 50)
        maxc = 50;
    // init connection pool
    this->pool = new MysqlConnectionPool(this->host, this->port, this->db, this->user, this->password, maxc);
}

mysqlpp::Connection *MysqlConnector::getConnection() {
    // direct get from connection pool
    return this->pool->grab();
}
