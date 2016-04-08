#include "mysqlconnectionpool.h"

#include <string>
#include <iostream>

using std::cout;
using std::string;

MysqlConnectionPool::MysqlConnectionPool(string host, unsigned int port, string db, string user, string pass, int maxConnection) :
    conns_in_use_(0),
    db_(db),
    server_(host),
    user_(user),
    password_(pass),
    port_(port),
    max_connections_(maxConnection)
{
}

MysqlConnectionPool::~MysqlConnectionPool() {
    clear();
}

// Do a simple form of in-use connection limiting: wait to return
// a connection until there are a reasonably low number in use
// already.  Can't do this in create() because we're interested in
// connections actually in use, not those created.  Also note that
// we keep our own count; ConnectionPool::size() isn't the same!
mysqlpp::Connection *MysqlConnectionPool::grab() {
    while (conns_in_use_ > 8) {
        // cout.put('R'); cout.flush(); // indicate waiting for release
        sleep(1);
    }

    ++conns_in_use_;
    return mysqlpp::ConnectionPool::grab();
}

void MysqlConnectionPool::release(const mysqlpp::Connection* pc) {
    mysqlpp::ConnectionPool::release(pc);
    --conns_in_use_;
}

// Superclass overrides
mysqlpp::Connection *MysqlConnectionPool::create() {
    // Create connection using the parameters we were passed upon
    // creation.  This could be something much more complex, but for
    // the purposes of the example, this suffices.
    // cout.put('C'); cout.flush(); // indicate connection creation
    return new mysqlpp::Connection(
            db_.empty() ? 0 : db_.c_str(),
            server_.empty() ? 0 : server_.c_str(),
            user_.empty() ? 0 : user_.c_str(),
            password_.empty() ? "" : password_.c_str());
}

// Our superclass can't know how we created the Connection, so
// it delegates destruction to us, to be safe.
void MysqlConnectionPool::destroy(mysqlpp::Connection* cp) {
    // cout.put('D'); cout.flush(); // indicate connection destruction
    delete cp;
}

// Set our idle time at an example-friendly 3 seconds.  A real
// pool would return some fraction of the server's connection
// idle timeout instead.
unsigned int MysqlConnectionPool::max_idle_time() {
    return max_connections_ < 0 ? 3 : max_connections_;
}

