#include "clientconnection.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <cstring>
#include <iostream>

#include "usersession.h"

using std::cout;
using std::endl;
using std::cerr;

ClientConnection::ClientConnection(int sockfd) {
    this->_sockfd = sockfd;
}
ClientConnection::~ClientConnection() {
    cout << "Destroy a connection." << endl;
    close(this->_sockfd);
}

void ClientConnection::startListener() {
    this->_listener_t = std::thread([=] { _doListen(); });
}

void ClientConnection::_doListen() {
    char buffer[1024];
    while(true) {
        bzero(buffer, 1024);
        int n = read(this->_sockfd, buffer, 1023);
        if (n < 0) {
            cerr << "ERROR reading from socket" << endl;
            break;
        } else if (n == 0) {
            cout << "client closes connection" << endl;
            break;
        }
        cout << "Here is the message: " << buffer << endl;
        n = write(this->_sockfd, "I got your message", 18);
        if (n < 0) cerr << "ERROR writing to socket" << endl;
    }
    // client disconnected
    close(this->_sockfd);
    UserSession::destroySession(this->_session->user()->id());
    cout << "ending client connection..." << endl;
    delete this;
}
