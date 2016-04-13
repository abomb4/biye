#include "usersession.h"
#include "config.h"

// static
map<unsigned int, UserSession*> UserSession::_uid_session_map;
map<int, UserSession*> UserSession::_sockfd_session_map;
BlockedMemoryPool *UserSession::_pool;

UserSession *UserSession::getSessionByUid(unsigned int id) {
    return UserSession::_uid_session_map[id];
}
UserSession *UserSession::getSessionBySockFd(int id) {
    return UserSession::_sockfd_session_map[id];
}

bool UserSession::createSession(ClientConnection *con, const User user) {
    if (user.id() == 0)
        return false;

    UserSession *newSession = new (UserSession::_pool->borrow(sizeof(UserSession))) UserSession(con, user);
    UserSession::_uid_session_map.insert(std::pair<unsigned int, UserSession *>(user.id(), newSession));
    UserSession::_sockfd_session_map.insert(std::pair<unsigned int, UserSession *>(con->sockfd(), newSession));

    return true;
}
bool UserSession::destroySessionByUid(unsigned int id, bool destroy_connection) {
    if (UserSession::_uid_session_map.count(id) == 0)
        return false;
    UserSession *us = UserSession::_uid_session_map[id];
    int sockfd = us->_connection->sockfd();

    if (destroy_connection) {
        us->_connection->~ClientConnection();
    }
    UserSession::_pool->restore((char *)us);
    UserSession::_uid_session_map.erase(id);
    UserSession::_sockfd_session_map.erase(sockfd);
}
bool UserSession::destroySessionBySockFd(int sockfd, bool destroy_connection) {
    if (UserSession::_sockfd_session_map.count(sockfd) == 0)
        return false;
    UserSession *us = UserSession::_sockfd_session_map[sockfd];
    unsigned int id = us->_user.id();

    if (destroy_connection) {
        us->_connection->~ClientConnection();
    }
    UserSession::_pool->restore((char *)us);
    UserSession::_uid_session_map.erase(id);
    UserSession::_sockfd_session_map.erase(sockfd);
}

// non-static
UserSession::UserSession(ClientConnection *con, const User u) {
    this->_user = u;
    this->_connection = con;
    int maxconn = atoi(Config::get("connection.max").c_str());
    if (maxconn < 10)
        maxconn = 1024;
    UserSession::_pool = new BlockedMemoryPool(sizeof(UserSession), maxconn);
}

UserSession::~UserSession() {
}
