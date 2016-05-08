#include "usersession.h"
#include "config.h"

#include "fxserver.h"

// static
map<unsigned int, UserSession*> UserSession::_uid_session_map;
map<int, UserSession*> UserSession::_sockfd_session_map;
BlockedMemoryPool *UserSession::_pool;

bool UserSession::initPool() {
    int maxconn = atoi(Config::get("connection.max").c_str());
    if (maxconn < 10)
        maxconn = 1024;
    UserSession::_pool = new BlockedMemoryPool(sizeof(UserSession), maxconn);
    return true;
}

UserSession *UserSession::getSessionByUid(unsigned int id) {
    return UserSession::_uid_session_map[id];
}
UserSession *UserSession::getSessionBySockFd(int id) {
    return UserSession::_sockfd_session_map[id];
}

bool UserSession::createSession(ClientConnection *con, const User user) {
    if (user.id() == 0)
        return false;

    UserSession *newSession = new (UserSession::_pool->borrow(sizeof(UserSession)))
            UserSession(con, user);
    newSession->toOnline();
    if (_uid_session_map.count(user.id()) > 0) {
        destroySessionByUid(user.id(), true);
    }
    UserSession::_uid_session_map.insert(std::pair<unsigned int, UserSession *>(user.id(), newSession));
    UserSession::_sockfd_session_map.insert(std::pair<unsigned int, UserSession *>(con->sockfd(), newSession));

    return true;
}
bool UserSession::destroySessionByUid(unsigned int id, bool destroy_connection) {
    if (UserSession::_uid_session_map.count(id) == 0)
        return false;
    UserSession *us = UserSession::_uid_session_map[id];

    if (destroy_connection) {
        if (us->_connection != nullptr) {
            us->_connection->closeSocket(); // ~ClientConnection();
        }
    }
    UserSession::_pool->restore((char *)us);
    UserSession::_uid_session_map.erase(id);
    if (us->_connection != nullptr) {
        int sockfd = us->_connection->sockfd();
        UserSession::_sockfd_session_map.erase(sockfd);
    }
    return true;
}
bool UserSession::destroySessionBySockFd(int sockfd, bool destroy_connection) {
    if (UserSession::_sockfd_session_map.count(sockfd) == 0)
        return false;
    UserSession *us = UserSession::_sockfd_session_map[sockfd];
    unsigned int id = us->_user.id();

    if (destroy_connection) {
        if (us->_connection != nullptr)
            us->_connection->~ClientConnection();
    }
    UserSession::_pool->restore((char *)us);
    UserSession::_uid_session_map.erase(id);
    UserSession::_sockfd_session_map.erase(sockfd);
    return true;
}
std::vector<ClientConnection*> *UserSession::getAllOnlineConnections() {
    std::vector<ClientConnection*> *v = new std::vector<ClientConnection*>();
    map<unsigned int, UserSession*>::iterator it = _uid_session_map.begin();
    while(it != _uid_session_map.end()) {
        if (!it->second->online()) {
            it++;
            continue;
        }
        ClientConnection *c = it->second->_connection;
        v->push_back(c);
        it++;
    }
    return v;
}

void UserSession::connectionDisconnected(int sockfd) {
    if (UserSession::_sockfd_session_map.count(sockfd) == 0)
        return;
    UserSession *us = UserSession::_sockfd_session_map[sockfd];
    us->toOffline();
    us->_connection = nullptr;
    UserSession::_sockfd_session_map.erase(sockfd);
}

std::vector<uint32_t> *UserSession::getOnlineList() {
    std::vector<uint32_t> *v = new std::vector<uint32_t>();
    map<unsigned int, UserSession*>::iterator it = _uid_session_map.begin();
    for (;it != _uid_session_map.end(); it++) {
        if ((*it).second->online())
            v->push_back((*it).first);
    }
    return v;
}

// non-static
UserSession::UserSession(ClientConnection *con, const User u) {
    this->_user = u;
    this->_connection = con;
    this->_confirmed = 0;
}

UserSession::~UserSession() {
}

void UserSession::confirmConnection() {
    if (this->_online) {
        _confirmed = 0;
        return;
    } else {
        _confirmed++;
        if (_confirmed == MAX_CONFIRM_FAIL_COUNT) {
            UserSession::destroySessionByUid(this->user().id(), true);
        }
    }
}

bool UserSession::online() {
    return this->_online;
}
void UserSession::toOnline() {
    this->_online = true;
    FxChat::FxServer::notifyOnline(this->user().id());
}
void UserSession::toOffline() {
    this->_online = false;
    FxChat::FxServer::notifyOffline(this->user().id());
}
