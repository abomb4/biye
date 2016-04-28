#include "fxconnection.h"

#include <QTcpSocket>
#include <QtEndian>

#include "linearmemorypool.h"
#include "config.h"

QMutex FxConnection::__connections_vector_using__;

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
FxConnection::FxConnection(const QString &host, const short &port, bool connects) {
    this->_host = host;
    this->_port = port;
    this->_pool = new LinearMemoryPool(FX_CONNECTION_POOL_SIZE);
    this->_socket = new QTcpSocket();
    connect(this->_socket, SIGNAL(connected()), this, SLOT(__connected_slot()));
    connect(this->_socket, SIGNAL(disconnected()), this, SLOT(__disconnected_slot()));
    this->created();
    if (connects) {
        this->_socket->connectToHost(this->_host, this->_port);
    }

    FxConnection::_new_connection_reg(this);
}

FxConnection::~FxConnection() {
    FxConnection::_remove_connection_reg(this);
    disconnect(this->_socket, SIGNAL(connected()), this, SLOT(__connected_slot()));
    disconnect(this->_socket, SIGNAL(disconnected()), this, SLOT(__disconnected_slot()));
    this->_socket->close();
    delete this->_socket;
    delete this->_pool;
    this->destoried();
}

///
/// \brief FxConnection::recieve 接收一个message
/// \param msgbuff
/// \return
///
FxChatError FxConnection::recieve(FxMessage *&msgbuff) {
    char *bodystr;
    uint32_t bodylength = 0;
    uint16_t fno = 0;
    FxChatError err = this->_doRead(bodystr, bodylength, fno);
    if (err != FxChatError::FXM_SUCCESS) { // wrong
        return err;
    } else {
        // parse msg
        err = this->_doParse(bodystr, bodylength, fno, msgbuff);
        return err;
    }
}

///
/// \brief FxConnection::send 发送一个message
/// \param x
/// \return
///
FxChatError FxConnection::send(const FxMessage *x) {
    char **packages;
    int *plengths;
    int package_sum = x->toPackages(packages, plengths, this->_pool);
    char *recvbuff = this->_pool->borrow(19);
    memset(recvbuff, 0, 19);
    for (int i = 0; i < package_sum; i++) {
        int n;
        int tries = 3;
        bool succ = false;
        while (tries--) {
            n = this->_socket->write(packages[i], plengths[i]);
            this->_socket->waitForBytesWritten();
            if (n <= 0) {
                return FxChatError::FXM_SOCKET_ERR;
            } else {
                succ = true;
                break;
            }
        }
        if (!succ)
            return FxChatError::FXM_SOCKET_ERR;
        this->_socket->waitForReadyRead(4000);
        n = this->_socket->read(recvbuff, 18);
        if (n < 0) {
            return FxChatError::FXM_NO_RESPONSE_RECIEVED;
        }
        if (strncmp(packages[i], recvbuff, 8) != 0
                || strncmp(recvbuff + 8, "stat:succ\n", 10) != 0) {
            qDebug() << "NON SUCC!";
        }
    }
    return FxChatError::FXM_SUCCESS;
}

char *FxConnection::borrowFromPool(int size) {
    return this->_pool->borrow(size);
}

void FxConnection::clearPool() {
    this->_pool->clear();
}
LinearMemoryPool *FxConnection::getPool() {
    return this->_pool;
}

bool FxConnection::connectToHost() {
    this->_socket->connectToHost(this->_host, this->_port);
    return this->_socket->waitForConnected(5000);
}

// private
FxChatError FxConnection::_doRead(char *&body, uint32_t &bodylength, uint16_t &fno, bool isrecursion) {
    int n = 0;
    char *buffer_8 = this->_pool->borrow(8);
    this->_socket->waitForReadyRead(4000);
    n = this->_socket->read(buffer_8, 8);

    // parse body length and from_user first
    uint16_t msg_body_length = 0;
    msg_body_length += buffer_8[0] << 8;
    msg_body_length += buffer_8[1];
    msg_body_length = qFromBigEndian(msg_body_length);

    uint16_t pagesize = 0;
    pagesize += buffer_8[2] << 8;
    pagesize += buffer_8[3];
    pagesize = qFromBigEndian(pagesize);

    uint16_t pageno = 0;
    pageno += buffer_8[4] << 8;
    pageno += buffer_8[5];
    pageno = qFromBigEndian(pageno);

    uint16_t _fno = 0;
    _fno += buffer_8[6] << 8;
    _fno += buffer_8[7];
    _fno = qFromBigEndian(_fno);

    char *buffer;
    if (!isrecursion) { // non-recursion, first call
        bodylength = msg_body_length * pagesize;
        body = this->_pool->borrow(bodylength);
        fno = _fno;
    }
    buffer = body;

    // recieve a body
    int need = msg_body_length;
    while(need > 0) {
        n = this->_socket->read(buffer, msg_body_length);
        if (n < 0) {
            return FxChatError::FXM_SOCKET_ERR;
        } else if (n == 0) {
            return FxChatError::FXM_SOCKET_ERR;
        }
        buffer += n;
        need -= n;
    } // body recieved

    // send recieved callback
    char ret[19];
    memset(ret, 0, 19);
    memcpy(ret, buffer_8, 8);
    memcpy(ret + 8, "stat:succ\n", 10);
    n = this->_socket->write(ret, 18);
    if (n < 0) {
        return FxChatError::FXM_SOCKET_ERR;
    } else if (n == 0) {
        return FxChatError::FXM_FAIL;
    }

    // read next package if exists
    if (pagesize > pageno) {
        FxChatError e = this->_doRead(buffer, bodylength, fno, true);
        if (e != FXM_SUCCESS) {
            return e;
        }
    } else if (pagesize > 1 && pagesize == pageno) {
        // recalculate bodylength
        bodylength -= (FXMESSAGE_BLOCK_SIZE - msg_body_length);
    }
    return FxChatError::FXM_SUCCESS;
}

FxChatError FxConnection::_doParse(char *body, uint32_t &bodylength, uint16_t &fno, FxMessage *&msg) {
    // START PARSE!
    msg = new (this->_pool->borrow(sizeof(FxMessage))) FxMessage();
    msg->fno(fno);
    FxMessageParam *p;
    char *line_cur = body; // line start
    char *colon_pos = line_cur; // ':' 's position
    char *nl_pos = line_cur; // '\n' 's position
    char current_stat = 0; // 0 finding ':'; 1 finding '\n';
    bool is_body = false;
    for (int i = bodylength; i > 0; i--) {
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
        int bodylen = bodylength - (colon_pos + 1 - body);
        p = new (this->_pool->borrow(sizeof(FxMessageParam))) FxMessageParam;
        p->setName(line_cur, (int)(colon_pos - line_cur - 1)); // minus ':'
        p->setVal(colon_pos + 1, bodylen);
        msg->addParam(p);
    }
    return FxChatError::FXM_SUCCESS;
}
