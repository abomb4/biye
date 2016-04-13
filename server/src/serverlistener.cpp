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
#include "blockedmemorypool.h"
#include "linearmemorypool.h"

using std::cerr;
using std::cout;
using std::endl;

ServerListener *ServerListener::s_instance = nullptr;
std::shared_ptr<spdlog::logger> ServerListener::_logger;

ServerListener *ServerListener::getInstance() {
    if (s_instance == nullptr)
        s_instance = new ServerListener();
    return s_instance;
}
ServerListener::ServerListener() {
    if (ServerListener::_logger == nullptr)
        ServerListener::_logger = spdlog::get("ServerListener");
    int maxconn = atoi(Config::get("connection.max").c_str());
    if (maxconn < 10)
        maxconn = 1024;
    this->_pool = new BlockedMemoryPool(sizeof(ClientConnection) + sizeof(LinearMemoryPool), maxconn);
}

ServerListener::~ServerListener() {
    delete (BlockedMemoryPool*) this->_pool;
}

void ServerListener::clear()
{
    // reuse TIME_WAIT port
    bool bReuseaddr = true;
    if (setsockopt(this->_sockfd, SOL_SOCKET ,SO_REUSEADDR,(const char*)&bReuseaddr,sizeof(bool)) < 0) {
        ServerListener::_logger->error("set SO_REUSEADDR failed.");
    }
    ServerListener::_logger->info("close socket.");
    close(this->_sockfd);
    s_instance->~ServerListener();
    s_instance = nullptr;
}

void ServerListener::restore(char *addr) {
    ServerListener::s_instance->_pool->restore(addr);
}

void ServerListener::startListener() {
    int port = atoi(Config::get("server.port").c_str());
    if (port == 0)
        port = 6777;

    // socket listen
    int newsockfd;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;
    int n;
    this->_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (this->_sockfd < 0) {
        cerr << "ERROR opening socket" << endl;
        exit(2);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY; // any IP
    serv_addr.sin_port = htons(port);

    // bind port
    if (bind(this->_sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        cerr << "ERROR on binding" << endl;
        close(this->_sockfd);
        exit(2);
        return;
    }
    listen(this->_sockfd, 5);

    clilen = sizeof(cli_addr);
    ClientConnection *c;
    int poolsize = atoi(Config::get("connection.pool.size").c_str());
    if (poolsize < 1024)
        poolsize = 1024;
    if (poolsize > 65536)
        poolsize = 65536;

    char *connection_base_addr;
    size_t size_per_connection = sizeof(ClientConnection) + sizeof(LinearMemoryPool);
    while(true) {
        ServerListener::_logger->info("waiting connection...");
        // recieve client link request
        newsockfd = accept(this->_sockfd, (struct sockaddr *) &cli_addr, &clilen);
        if (newsockfd < 0)
            ServerListener::_logger->error("ERROR on accept: {}", newsockfd);
        ServerListener::_logger->info("accepted an client.");
        connection_base_addr = this->_pool->borrow(size_per_connection);
        c = new (connection_base_addr) ClientConnection(
                    newsockfd,
                    poolsize,
                    connection_base_addr + sizeof(ClientConnection));
        c->setRestoreFunc(&ServerListener::restore);
        c->startListener();
    }
}
