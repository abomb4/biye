#ifndef USERMANAGER_H
#define USERMANAGER_H

#include "user.h"
#include "memorypool.h"

class UserManager
{
public:
    // Get singleton class instance
    static UserManager &getInstance();

    // login
    bool login(const char *name, const char *password, uint32_t &userid);

    // modify by id
    bool modify(User u);

    // check userid exists
    bool exists(uint32_t userid);

    ///
    /// \brief getList
    /// \param u condition
    /// \param pageno if 0 will return all users
    /// \param pagesize if 0 will return all users
    /// \param return_users will point to memory alloctaed from pool
    /// \param pool
    /// \return num of users
    ///
    int getFullList(User *&return_users, MemoryPool *pool);

private:
    static UserManager *s_instance;
    UserManager();
};

#endif // USERMANAGER_H
