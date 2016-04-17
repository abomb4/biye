#include "fxconnection.h"

#include <QTcpSocket>
#include <QtEndian>

#include "linearmemorypool.h"
#include "config.h"

#include <iostream>
using namespace std;

QMutex __connections_vector_using__;

// static
QVector<FxConnection*> FxConnection::_connections;

FxConnection *__server_connection__ = nullptr;
FxConnection *FxConnection::getServerConnection() {
    if (__server_connection__ == nullptr) { // connect to server firstly
        QString host = Config::get("server.host");
        short port = Config::get("server.port").toInt();
        if (port <= 0)
            port = 6777;
        __server_connection__ = new FxConnection(host, port);
    } else if (!_connections.contains(__server_connection__)) {
        QString host = Config::get("server.host");
        short port = Config::get("server.port").toInt();
        if (port <= 0)
            port = 6777;
        __server_connection__ = new FxConnection(host, port);
    }
    return __server_connection__;
}

void FxConnection::_new_connection_reg(FxConnection *c) {
    __connections_vector_using__.lock();
    if (_connections.contains(c))
        return;
    _connections.append(c);
    __connections_vector_using__.unlock();
}

void FxConnection::_remove_connection_reg(FxConnection *c) {
    __connections_vector_using__.lock();
    int i = _connections.indexOf(c);
    if (i < 0)
        return;
    _connections.remove(i);
    __connections_vector_using__.unlock();
}

// non static
FxConnection::FxConnection(const QString host, const short port) {
    this->_pool = new LinearMemoryPool(FX_CONNECTION_POOL_SIZE);
    this->_socket = new QTcpSocket();
    this->_socket->connectToHost(host, port);

    this->created();

    FxConnection::_new_connection_reg(this);
}

FxConnection::~FxConnection() {
    FxConnection::_remove_connection_reg(this);
    this->destoried();
    delete this->_socket;
}


FxChatError FxConnection::recieve(FxMessage *&msgbuff) {
    char *buffer = this->_pool->borrow(8);
    bzero(buffer, 8);
    this->_socket->waitForReadyRead(4000);
    int n = this->_socket->read(buffer, 8);
    if (n < 0) {
        return FxChatError::FXM_SOCKET_ERR;
    } else if (n == 0) {
        this->disconnected();
        return FxChatError::FXM_SOCKET_ERR;
    }
    if (n < 8) {
        return FxChatError::FXM_MSG_TOO_SHORT;
    } else {
        return FxConnection::_doParse(buffer, msgbuff);
    }
}

FxChatError FxConnection::send(const FxMessage *x) {
    int buffer_length = x->needBufferSize();
    char *buffer = this->_pool->borrow(buffer_length);
    if (!x->toCharStr(buffer, buffer_length)) {
        // ERROR convert FxMesasge to char str
        int n = this->_socket->write("failed to build msg", 19);
        return FxChatError::FXM_PARSE_FAIL;
    } else {
        int n = 0;
        n = this->_socket->write(buffer, buffer_length);
        if (n < 0) {
            return FxChatError::FXM_SOCKET_ERR;
        }
    }
    return FxChatError::FXM_SUCCESS;
}

void FxConnection::startReadThread() {

}

char *FxConnection::borrowFromPool(int size) {
    return this->_pool->borrow(size);
}

void FxConnection::clearPool() {

}

// private
FxChatError FxConnection::_doParse(const char *buffer_8, FxMessage *&msg) {
    // parse body length and from_user first
    uint16_t msg_body_length = 0;
    msg_body_length += buffer_8[0] << 8;
    msg_body_length += buffer_8[1];
    msg_body_length = qFromBigEndian(msg_body_length);
    if (msg_body_length > MAX_FXMESSAGE_SIZE - 8) { // message body too long
        return FxChatError::FXM_MSG_TOO_LONG;
    }
    msg = new (this->_pool->borrow(sizeof(FxMessage))) FxMessage();

    uint32_t from_user_n = 0;
    from_user_n += buffer_8[2] << 24;
    from_user_n += buffer_8[3] << 16;
    from_user_n += buffer_8[4] << 8;
    from_user_n += buffer_8[5];
    msg->fromUser(qFromBigEndian(from_user_n));

    uint16_t fno = 0;
    fno += buffer_8[6] << 8;
    fno += buffer_8[7];
    msg->fno(qFromBigEndian(fno));

    // do read
    char *msg_body_buff = this->_pool->borrow(msg_body_length);
    int n = 0, need = msg_body_length;
    while(need > 0) {
        n = this->_socket->read(msg_body_buff + msg_body_length - need, msg_body_length);
        if (n < 0) {
            cout << this->_socket->error() << endl;
            return FxChatError::FXM_SOCKET_ERR;
        } else if (n == 0) {
            return FxChatError::FXM_SOCKET_ERR;
        }
        need -= n;
    }

    // START PARSE!
    FxMessageParam *p;
    char *line_cur = msg_body_buff; // line start
    char *colon_pos = line_cur; // ':' 's position
    char *nl_pos = line_cur; // '\n' 's position
    char current_stat = 0; // 0 finding ':'; 1 finding '\n';
    bool is_body = false;
    for (int i = msg_body_length; i > 0; i--) {
        // loop use nl_pos
        if (current_stat == 0 && *nl_pos == ':') { // ':' founded
            if (line_cur == nl_pos) { // a line start with ':'
                return FxChatError::FXM_PARSE_FAIL;
            }
            colon_pos = nl_pos;
            if (*line_cur == 'b') {
                if (*(line_cur + 1) == 'o' && *(line_cur + 2) == 'd' && *(line_cur + 3) == 'y')
                    is_body = true;
            }
            current_stat = 1;
        } else if (current_stat == 1 && *nl_pos == '\n') { // '\n' founded
            // do parse line
            p = new (this->_pool->borrow(sizeof(FxMessageParam))) FxMessageParam;
            p->setName(line_cur, (int)(colon_pos - line_cur)); //
            p->setVal(colon_pos + 1, (int)(nl_pos - colon_pos - 1)); // '\n'
            msg->addParam(p);

            p = nullptr;
            current_stat = 0;
            line_cur = nl_pos + 1;
            colon_pos = line_cur;
            is_body = false;
        }
        if (is_body)
            break;
        nl_pos++;
    }
    // parse body
    if (is_body) {
        int bodylen = msg_body_length - (colon_pos + 1 - msg_body_buff);
        p = new (this->_pool->borrow(sizeof(FxMessageParam))) FxMessageParam;
        p->setName(line_cur, (int)(colon_pos - line_cur - 1)); // minus ':'
        p->setVal(colon_pos + 1, bodylen);
        msg->addParam(p);
    }
    return FxChatError::FXM_SUCCESS;
}
