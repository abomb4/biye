#include "usermanager.h"

#include "mysqlconnector.h"
#include <iostream>
using std::cerr;
using std::cout;
using std::endl;

UserManager *UserManager::s_instance;

UserManager::UserManager() {
}

UserManager &UserManager::getInstance() {
    if (UserManager::s_instance == nullptr) {
        UserManager::s_instance = new UserManager();
    }
    return *s_instance;
}

bool UserManager::login(const char *username, const char *password, uint32_t &userid) {
    bool result = false;
    mysqlpp::Connection *con = MysqlConnector::getInstance()->getConnection();
    mysqlpp::Query q = con->query();
    q << "select id, name from fx_user where name = %0q and password = %1q";
    q.parse();
    try {
        mysqlpp::StoreQueryResult res = q.store(username, password);
        if (res.size() < 1) {
            result = false;
        } else {
            result = true;
            mysqlpp::Row row = res.begin()[0];
            userid = atoi(((string)row[0]).c_str());
        }
    } catch (const mysqlpp::BadQuery& er) {
        // Handle any query errors
        cerr << "Query error: " << er.what() << endl;
        result = false;
    } catch (const mysqlpp::BadConversion& er) {
        // Handle bad conversions
        cerr << "Conversion error: " << er.what() << endl <<
                "\tretrieved data size: " << er.retrieved <<
                ", actual size: " << er.actual_size << endl;
        result = false;
    } catch (const mysqlpp::Exception& er) {
        // Catch-all for any other MySQL++ exceptions
        cerr << "Error: " << er.what() << endl;
        result = false;
    }
    MysqlConnector::getInstance()->release(con);
    return result;
}

int UserManager::getFullList(User *&return_users, MemoryPool *pool) {
    int count = 0;
    mysqlpp::Connection *con = MysqlConnector::getInstance()->getConnection();
    mysqlpp::Query q = con->query();
    q << "select id, name, department, true_name from fx_user where status = 1";
    try {
        mysqlpp::StoreQueryResult res = q.store();
        if (!res.empty()) {
            count = res.size();
            return_users = new (pool->borrow(sizeof(User) * count)) User[count]();
            mysqlpp::StoreQueryResult::const_iterator it;
            int i;
            for (i = 0, it = res.begin(); it != res.end(); ++it, ++i) {
                mysqlpp::Row row = *it;
                return_users[i].id(atoi(((string)row[0]).c_str()));
                return_users[i].name(((string)row[1]).c_str());
                return_users[i].department(atoi(((string)row[2]).c_str()));
                return_users[i].trueName(((string)row[3]).c_str());
            }
        }
    } catch (const mysqlpp::BadQuery& er) {
        // Handle any query errors
        cerr << "Query error: " << er.what() << endl;
    } catch (const mysqlpp::BadConversion& er) {
        // Handle bad conversions
        cerr << "Conversion error: " << er.what() << endl <<
                "\tretrieved data size: " << er.retrieved <<
                ", actual size: " << er.actual_size << endl;
    } catch (const mysqlpp::Exception& er) {
        // Catch-all for any other MySQL++ exceptions
        cerr << "Error: " << er.what() << endl;
    }
    MysqlConnector::getInstance()->release(con);
    return count;
}
