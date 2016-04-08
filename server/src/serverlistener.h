#ifndef SERVERLISTENER_H
#define SERVERLISTENER_H

/**
 * 单入口Listener，用于监听端口，接收所有客户端请求。
 * @brief The ServerListener class
 */
class ServerListener
{
public:
    // Get singleton class instance
    static ServerListener &getInstance();

    // Clear action before service quit
    void clear();

    // START SERVICE!
    void startListener();

private:
    static ServerListener s_instance;
    ServerListener();

    int sockfd;
};

#endif // SERVERLISTENER_H
