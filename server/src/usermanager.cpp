#include "usermanager.h"

#include <iostream>

#include "mysqlconnector.h"

using std::cout;
using std::cerr;
using std::endl;

UserManager *UserManager::s_instance;

UserManager::UserManager() {

}

UserManager &UserManager::getInstance() {
    if (UserManager::s_instance == 0) {
        UserManager::s_instance = new UserManager();
    }
    return *s_instance;
}

bool UserManager::login(User u) {
    mysqlpp::Connection *con = MysqlConnector::getInstance().getConnection();
    mysqlpp::Query q = con->query();
    q << "select id, name, password, email from user where name = %0q and password = %1q";
    q.parse();
    try {
        mysqlpp::StoreQueryResult res = q.store(u.name(), u.password());
        if (res.size() < 1) {
            return false;
        } else {
            mysqlpp::Row row = res[0];
            cout << "id: " << row[0]
                 << ", name: " << row[1]
                 << ", password: " << row[2]
                 << ", email: " << row[3]
                 << "." << endl;
            return true;
        }
    } catch (const mysqlpp::BadQuery& er) {
        // Handle any query errors
        cerr << "Query error: " << er.what() << endl;
        return false;
    } catch (const mysqlpp::BadConversion& er) {
        // Handle bad conversions
        cerr << "Conversion error: " << er.what() << endl <<
                "\tretrieved data size: " << er.retrieved <<
                ", actual size: " << er.actual_size << endl;
        return false;
    } catch (const mysqlpp::Exception& er) {
        // Catch-all for any other MySQL++ exceptions
        cerr << "Error: " << er.what() << endl;
        return false;
    }

}
