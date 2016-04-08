#include "usersession.h"

// static
map<unsigned int, UserSession*> UserSession::_sessions;

UserSession *UserSession::getSession(unsigned int id) {
    return UserSession::_sessions[id];
}

bool UserSession::createSession(ClientConnection *con, User *user) {
    if (user->id() == 0)
        return false;

    UserSession *newSession = new UserSession(con, user);
    UserSession::_sessions.insert(std::pair<unsigned int, UserSession *>(user->id(), newSession));
    return true;
}
bool UserSession::destroySession(unsigned int id) {
    if (UserSession::_sessions.count(id) == 0)
        return false;
    delete UserSession::_sessions[id];
    UserSession::_sessions.erase(id);

}

// non-static
UserSession::UserSession(ClientConnection *con, User *u) {
    this->_user = u;
    this->_connection = con;
}

UserSession::~UserSession() {
    delete this->_user;
}

const User *UserSession::user() const {
    return this->_user;
}
