#include "serverlistener.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <iostream>
#include <string>
#include <cstdlib>
#include <cstring>

#include "config.h"
#include "clientconnection.h"

using std::cerr;
using std::cout;
using std::endl;

ServerListener ServerListener::s_instance;

ServerListener &ServerListener::getInstance() {
    return s_instance;
}
ServerListener::ServerListener()
{
}
void ServerListener::clear()
{
    // reuse TIME_WAIT port
    bool bReuseaddr = true;
    if (setsockopt(this->sockfd, SOL_SOCKET ,SO_REUSEADDR,(const char*)&bReuseaddr,sizeof(bool)) < 0) {
        cerr << "set SO_REUSEADDR failed." << endl;
    }
    cout << "close" << endl;
    close(this->sockfd);
}

void ServerListener::startListener() {
    int port = atoi(Config::get("server.port").c_str());
    if (port == 0)
        port = 6777;

    // socket listen
    int newsockfd;
    socklen_t clilen;
    char buffer[256];
    struct sockaddr_in serv_addr, cli_addr;
    int n;
    this->sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (this->sockfd < 0)
        cerr << "ERROR opening socket" << endl;
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY; // any IP
    serv_addr.sin_port = htons(port);

    // bind port
    if (bind(this->sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        cerr << "ERROR on binding" << endl;
        close(this->sockfd);
        return;
    }
    listen(this->sockfd, 5);
    clilen = sizeof(cli_addr);
    ClientConnection *c;
    while(true) {
        cout << "waiting connection..." << endl;
        // recieve client link request
        newsockfd = accept(this->sockfd, (struct sockaddr *) &cli_addr, &clilen);
        if (newsockfd < 0)
            cerr << "ERROR on accept" << endl;
        cout << "accepted an client." << endl;
        c = new ClientConnection(newsockfd);
        c->startListener();
    }
}
