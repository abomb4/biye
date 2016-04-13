#ifndef CLIENTCONNECTION_H
#define CLIENTCONNECTION_H

#include <iostream>
#include <thread> // C++11

#include "fxmessage.h"
using namespace FxChat;

#include "memorypool.h"

#include <spdlog/spdlog.h>

class ClientConnection
{
public:
    ClientConnection(int sockfd, int poolsize, char *poolObjMemory);
    ~ClientConnection();

    void setRestoreFunc(void (*func)(char *));

    MemoryPool *pool() { return this->_pool; }
    const int sockfd() const { return this->_sockfd; }

    void startListener();
private:
    static std::shared_ptr<spdlog::logger> _logger;
    int _sockfd; // socket
    std::thread _listener_t;
    std::mutex _send_mutex;

    // start listener thread, will do 'delete(this)' at end
    void _doListen();

    // parse msg
    FxChatError _doParse(const char *buffer_8, FxMessage *&msg);

    // send msg
    void _doSend(FxMessage *x);

    // default pool
    MemoryPool *_pool;

    // restore memory
    void (*_restore)(char *addr);
};

#endif // CLIENTCONNECTION_H
