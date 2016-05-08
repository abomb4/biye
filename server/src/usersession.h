#ifndef USERSESSION_H
#define USERSESSION_H

#include <map>
#include <vector>

#include "blockedmemorypool.h"
#include "user.h"
#include "clientconnection.h"

#define MAX_CONFIRM_FAIL_COUNT 6

class ClientConnection;

using std::map;

class UserSession
{
public:
    static bool initPool();
    static UserSession *getSessionByUid(unsigned int id);
    static UserSession *getSessionBySockFd(int id);
    static bool createSession(ClientConnection *con, const User user);
    static bool destroySessionByUid(unsigned int id, bool destroy_connection);
    static bool destroySessionBySockFd(int id, bool destroy_connection);

    static std::vector<uint32_t> *getOnlineList();
    static void connectionDisconnected(int sockfd);

    static std::vector<ClientConnection *> *getAllOnlineConnections();

    // destruct method dont free *_connection
    ~UserSession();

    const User user() const { return this->_user; }
    ClientConnection *connection() const { return this->_connection; }

    bool online();
    void toOnline();
    void toOffline();

    void confirmConnection();

private:
    static map<unsigned int, UserSession*> _uid_session_map;
    static map<int, UserSession*> _sockfd_session_map;

    static BlockedMemoryPool *_pool;

    UserSession(ClientConnection *con, const User u);

    ClientConnection *_connection; // pointer to obj
    User _user;
    char *_sessionid;
    bool _online;
    unsigned short _confirmed;
};

#endif // USERSESSION_H
