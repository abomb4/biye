#ifndef SERVERLISTENER_H
#define SERVERLISTENER_H

#include "memorypool.h"

#include <spdlog/spdlog.h>

///
/// 单入口Listener，用于监听端口，接收所有客户端请求。
/// \brief The ServerListener class
///
class ServerListener
{
public:
    // Get singleton class instance
    static ServerListener *getInstance();
    ~ServerListener();

    // Clear action before service quit
    void clear();

    // START SERVICE!
    void startListener();

    // Restore to pool, uses for ClientConnection
    static void restore(char *addr);
private:
    static ServerListener *s_instance;
    static std::shared_ptr<spdlog::logger> _logger;
    ServerListener();

    int _sockfd;
    MemoryPool *_pool; // store ClientConnection object
};

#endif // SERVERLISTENER_H
