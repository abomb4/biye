#ifndef MYSQLCONNECTIONPOOL_H
#define MYSQLCONNECTIONPOOL_H

#define MYSQLPP_MYSQL_HEADERS_BURIED

#include <string>
#include <unistd.h>

#include <mysql++/mysql++.h>

using std::string;

///
/// \brief The MysqlConnectionPool class, copied from mysql++ webset.
/// \link http://tangentsoft.net/mysql++/doc/html/userman/threads.html
///
class MysqlConnectionPool : public mysqlpp::ConnectionPool
{
public:
    // The object's only constructor
    MysqlConnectionPool(string host, unsigned int port, string db, string user, string pass, int maxConnection);

    // The destructor.  We _must_ call ConnectionPool::clear() here,
    // because our superclass can't do it for us.
    ~MysqlConnectionPool();

    // Do a simple form of in-use connection limiting: wait to return
    // a connection until there are a reasonably low number in use
    // already.  Can't do this in create() because we're interested in
    // connections actually in use, not those created.  Also note that
    // we keep our own count; ConnectionPool::size() isn't the same!
    mysqlpp::Connection* grab();

    // Other half of in-use conn count limit
    void release(const mysqlpp::Connection* pc);

protected:
    // Superclass overrides
    mysqlpp::Connection* create();

    void destroy(mysqlpp::Connection* cp);
    unsigned int max_idle_time();

private:
    // Number of connections currently in use
    unsigned int conns_in_use_;

    // Our connection parameters
    string db_, server_, user_, password_;
    unsigned int port_;

    // Maximum connections in pool
    int max_connections_;

};
#endif // MYSQLCONNECTIONPOOL_H
