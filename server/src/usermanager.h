#ifndef USERMANAGER_H
#define USERMANAGER_H

#include <vector>
#include "user.h"

using std::vector;

class UserManager
{
public:
    // Get singleton class instance
    static UserManager &getInstance();

    // login
    bool login(const char *name, const char *password);

    // modify by id
    bool modify(User u);

    // get user list
    vector<User> getList(User u, int pageno, int pagesize);
private:
    static UserManager *s_instance;
    UserManager();
};

#endif // USERMANAGER_H
