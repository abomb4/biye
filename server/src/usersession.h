#ifndef USERSESSION_H
#define USERSESSION_H

#include <map>

#include "user.h"
#include "clientconnection.h"

class ClientConnection;

using std::map;

class UserSession
{
public:
    static UserSession *getSession(unsigned int id);
    static bool createSession(ClientConnection *con, User *user);
    static bool destroySession(unsigned int id);

    // destruct method dont free *_connection
    ~UserSession();

    const User *user() const;

private:
    static map<unsigned int, UserSession*> _sessions;

    UserSession(ClientConnection *con, User *u);
    ClientConnection *_connection;
    const User *_user;
};

#endif // USERSESSION_H
