#ifndef FXCONNECTION_H
#define FXCONNECTION_H

#include <QThread>
#include <QMutex>
#include <QVector>
#include <QTcpSocket>

#include "linearmemorypool.h"
#include "fxmessage.h"

#define FX_CONNECTION_POOL_SIZE 2560
#define FX_CONNECTION_HEARTBEAT_INTERVAL 5 // minute
#define FX_CONNECTION_READ_INTERVAL 2 // second

using namespace FxChat;

class FxConnection : public QObject
{
    Q_OBJECT
public:
    static FxConnection *getServerConnection();

    FxConnection(const QString &host, const short &port, bool connect = true);
    ~FxConnection();

    bool isConnectetd() { return this->_socket->state() == QAbstractSocket::SocketState::ConnectedState; };
    void lock() { this->_using_mutex.lock(); }
    void unlock() { this->_using_mutex.unlock(); }

    FxChatError recieve(FxMessage *&buffer);
    FxChatError send(const FxMessage *data);

    bool connectToHost();

    // DON'T FREE/DELETE BORROWED MEMORY!! USE clearPool() TO RESTORE ALL BORROWED MEM!!
    char *borrowFromPool(int size);
    LinearMemoryPool *getPool();
    void clearPool();

    enum ConnectionType { TCP_LONG_LINK, UDP_DIRECT };

signals:
    void created();
    void destoried();
    void connected();
    void disconnected();

private:
    static QVector<FxConnection*> _connections;
    static QMutex __connections_vector_using__;

    void _read_thread_worker();
    // read a full msg string
    FxChatError _doRead(char *&body, uint32_t &bodylength, uint16_t &fno, bool isrecursion = false);
    // parse
    FxChatError _doParse(char *body, uint32_t &bodylength, uint16_t &fno, FxMessage *&msg);

    // QAbstractSocket *_socket;
    QTcpSocket *_socket;
    LinearMemoryPool *_pool;
    QMutex _using_mutex;
    QString _host;
    short _port;

public slots:
    static void _remove_connection_reg(FxConnection *);
    static void _new_connection_reg(FxConnection *);

    void __connected_slot() { this->connected(); }
    void __disconnected_slot() { this->disconnected(); }
};

#endif // FXCONNECTION_H
