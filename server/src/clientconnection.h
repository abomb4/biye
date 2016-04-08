#ifndef CLIENTCONNECTION_H
#define CLIENTCONNECTION_H

#include <iostream>
#include <thread> // C++11

#include "fxmessage.h"
#include "usersession.h"

class UserSession;

class ClientConnection
{
public:
    ClientConnection(int sockfd);
    ~ClientConnection();
    void startListener();

private:
    int _sockfd; // socket
    std::thread _listener_t;

    // session pointer
    const UserSession *_session;

    // start listener thread, will do 'delete(this)' at end
    void _doListen();

    // parse msg, caller should call delete manually
    FxMessage *_doParse();

    // send msg
    void _doSend(const FxMessage *x);
};

#endif // CLIENTCONNECTION_H
