#ifndef FXCONNECTION_H
#define FXCONNECTION_H

#include <QThread>
#include <QMutex>
#include <QVector>
#include <QAbstractSocket>

#include "memorypool.h"
#include "fxmessage.h"

#define FX_CONNECTION_POOL_SIZE 2560
using namespace FxChat;

class FxConnection
{
public:
    static FxConnection *getServerConnection();

    FxConnection(const QString host, const short port);
    ~FxConnection();

    bool isConnectetd() { return this->_socket->state() == QAbstractSocket::SocketState::ConnectedState; };
    void lock() { this->_using_mutex.lock(); }
    void unlock() { this->_using_mutex.unlock(); }

    FxChatError recieve(FxMessage *&buffer);
    FxChatError send(const FxMessage *data);

    void startReadThread();

    char *borrowFromPool(int size);
    void clearPool();

signals:
    void created() {}
    void destoried() {}
    void recieved() {}
    void disconnected() {}

private:
    static QVector<FxConnection*> _connections;

    void _do_read_thread();
    FxChatError _doParse(const char *buffer_8, FxMessage *&msg);

    QThread _thread;
    QAbstractSocket *_socket;
    MemoryPool *_pool;
    QMutex _using_mutex;

private slots:
    static void _remove_connection_reg(FxConnection *);
    static void _new_connection_reg(FxConnection *);
};

#endif // FXCONNECTION_H
