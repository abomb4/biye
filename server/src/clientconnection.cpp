#include "clientconnection.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <cstring>
#include <iostream>

#include "config.h"
#include "linearmemorypool.h"
#include "fxserver.h"
#include "usersession.h"

using std::cout;
using std::endl;
using std::cerr;
using std::mutex;

ssize_t _fx_tcp_send(int __fd, const void *__buf, size_t __n, int __flags) {
    ssize_t n = 0;
    n += send(__fd, __buf, 8, __flags);
    n += send(__fd, (char*)__buf + 8, __n - 8, __flags);
    return n;
}

void dummy(char *a){};
std::shared_ptr<spdlog::logger> ClientConnection::_logger;

ClientConnection::ClientConnection(int sockfd, int poolsize, char *poolObjMemory) {
    if (ClientConnection::_logger == nullptr)
        ClientConnection::_logger = spdlog::get("ClientConnection");
    this->_sockfd = sockfd;
    this->_pool = new (poolObjMemory) LinearMemoryPool(poolsize);
    this->_restore = dummy;
    ClientConnection::_logger->info("({}) A ClientConnection object created, socket id:{}",
                              (void*)this, this->_sockfd);
}

ClientConnection::~ClientConnection() {
    ClientConnection::_logger->info("({}) A connection destoried.", (void*)this);
    //UserSession::destroySessionBySockFd(this->_sockfd, false); // TODO this may be unexpected disconnection
    UserSession::connectionDisconnected(this->_sockfd);
    this->_listener_t.~thread();
    close(this->_sockfd);
    // CANNOT DELETE BECAUSE OF BLOCKEDMEMORYPOOL
    LinearMemoryPool *_free = (LinearMemoryPool *)this->_pool;
    _free->~LinearMemoryPool();
    // delete this->_pool;

    // free from blockedMemoryPool
    (*this->_restore)((char *)this);
}

void ClientConnection::setRestoreFunc(void (*func)(char *)) {
    this->_restore = *func;
}

void ClientConnection::startListener() {
    this->_listener_t = std::thread([=] { _doListen(); });
    this->_listener_t.detach();
}

void ClientConnection::_doListen() {
    char buffer[8];
    bool _breakIt = false;
    while(true) {
        // buffer = this->_pool->borrow(8);
        memset(buffer, 0, 8);
        // recieve msg head, MSG_PEEK mode
        int n = recv(this->_sockfd, buffer, 8, MSG_PEEK);
        this->_send_mutex.lock(); // LOCK
        if (n < 0) {
            ClientConnection::_logger->error("({}) ERROR reading from socket", (void*)this);
            this->unlock();
            this->~ClientConnection();
            return;
        } else if (n == 0) {
            ClientConnection::_logger->info("({}) client closes connection", (void*)this);
            this->unlock();
            this->~ClientConnection();
            return;
        }
        FxMessage *returnMsg = nullptr;
        // less than 8Byte message
        if (n < 8) {
            recv(this->_sockfd, buffer, 8, 0);
            FxServer::makeFailureMsg(returnMsg, FxChatError::FXM_MSG_TOO_SHORT, this->_pool, 0);
            this->_doSend(returnMsg);
            this->_send_mutex.unlock();
            this->~ClientConnection();
            return;
        } else { // normal
            FxMessage *recieveMsg = nullptr;
            char *bodystr;
            uint32_t bodylength = 0;
            uint16_t fno = 0;
            FxChatError err = this->_doRead(bodystr, bodylength, fno);
            if (err != FxChatError::FXM_SUCCESS) { // wrong
                if (err == FxChatError::FXM_SOCKET_ERR) {
                    this->unlock();
                    this->~ClientConnection();
                    return;
                }
                FxServer::makeFailureMsg(returnMsg, err, this->_pool, 0);
            } else {
                // parse msg
                err = this->_doParse(bodystr, bodylength, fno, recieveMsg);
                if (err != FXM_SUCCESS) {
                    FxServer::makeFailureMsg(returnMsg, err, this->_pool, 0);
                } else {
                    // LETS DO OPERATION!
                    err = FxServer::doOperation(returnMsg, recieveMsg, this);
                    if (err != FxChatError::FXM_SUCCESS) {
                        FxServer::makeFailureMsg(returnMsg, err, this->_pool);
                        if (err == FxChatError::FXM_MSG_TOO_LONG || err == FxChatError::FXM_PARSE_FAIL)
                            _breakIt = true;
                    }
                }
            }
        }
        this->_doSend(returnMsg);
        this->_send_mutex.unlock(); // unlock
        this->_pool->clear();

        if (_breakIt)
            break;
    }
    // client disconnected
    // CANNOT DELETE BECAUSE OF BLOCKEDMEMORYPOOL
    this->~ClientConnection();
    // delete this;
}

FxChatError ClientConnection::_doRead(char *&body, uint32_t &bodylength, uint16_t &fno, bool isrecursion) {
    int n = 0;
    char *buffer_8 = this->_pool->borrow(8);
    n = recv(this->_sockfd, buffer_8, 8, 0);
    // parse body length and from_user first
    uint16_t msg_body_length = 0;
    msg_body_length += buffer_8[0] << 8;
    msg_body_length += buffer_8[1];
    msg_body_length = ntohs(msg_body_length);

    uint16_t pagesize = 0;
    pagesize += buffer_8[2] << 8;
    pagesize += buffer_8[3];
    pagesize = ntohs(pagesize);

    uint16_t pageno = 0;
    pageno += buffer_8[4] << 8;
    pageno += buffer_8[5];
    pageno = ntohs(pageno);

    uint16_t _fno = 0;
    _fno += buffer_8[6] << 8;
    _fno += buffer_8[7];
    _fno = ntohs(_fno);

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
        n = recv(this->_sockfd, buffer, msg_body_length, 0);
        if (n < 0) {
            ClientConnection::_logger->error("({}) ERROR reading from socket in _doParse()", (void*)this);
            return FxChatError::FXM_SOCKET_ERR;
        } else if (n == 0) {
            ClientConnection::_logger->info("({}) client closes connection in _doParse()", (void*)this);
            return FxChatError::FXM_SOCKET_ERR;
        }
        buffer += n;
        need -= n;
    } // body recieved
/*
    // send recieved callback
    char ret[19];
    memset(ret, 0, 19);
    memcpy(ret, buffer_8, 8);
    memcpy(ret + 8, "stat:succ\n", 10);
    n = write(this->_sockfd, ret, 18);
    if (n < 0) {
        ClientConnection::_logger->error("({}) ERROR writing from socket in _doParse()", (void*)this);
        return FxChatError::FXM_SOCKET_ERR;
    } else if (n == 0) {
        ClientConnection::_logger->info("({}) client closes connection in _doParse()", (void*)this);
        return FxChatError::FXM_SOCKET_ERR;
    }
*/
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

FxChatError ClientConnection::_doParse(char *body, uint32_t bodylength, uint16_t fno, FxMessage *&msg) {
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
                _logger->info("({}) PARSE FAIL! MSG HAVE A LINE START WITH ':'!", (void*)this);
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
        p->setName(line_cur, (int)(colon_pos - line_cur)); // minus ':'
        p->setVal(colon_pos + 1, bodylen);
        msg->addParam(p);
    }
    return FxChatError::FXM_SUCCESS;
}

void ClientConnection::_doSend(const FxMessage *x) {
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
            n = _fx_tcp_send(this->_sockfd, packages[i], plengths[i], MSG_NOSIGNAL);
            if (n <= 0) {
                ClientConnection::_logger->error("({}) ERROR writing to socket ({})", (void*)this, errno);
                succ = false;
            } else {
                succ = true;
                break;
            }
        }
        if (!succ)
            return;
        /*
        n = recv(this->_sockfd, recvbuff, 18, 0);
        if (n < 0) {
            ClientConnection::_logger->error("({}) ERROR reading response pack.", (void*)this);
            break;
        }
        if (strncmp(packages[i], recvbuff, 8) != 0
                || strncmp(recvbuff + 8, "stat:succ\n", 10) != 0) {
            ClientConnection::_logger->error("({}) ERROR WRONG RESPONSE HEADER!!", (void*)this);
        }
        */
    }
}

void ClientConnection::lock() {
    this->_send_mutex.lock(); // LOCK
}

void ClientConnection::unlock() {
    this->_send_mutex.unlock(); // LOCK
}

FxChatError ClientConnection::readMsg(FxMessage *&msg) {
    char *bodystr;
    uint32_t bodylength = 0;
    uint16_t fno = 0;
    FxChatError err = this->_doRead(bodystr, bodylength, fno);
    if (err != FxChatError::FXM_SUCCESS) { // wrong
        return err;
    } else {
        // parse msg
        err = this->_doParse(bodystr, bodylength, fno, msg);
        if (err != FXM_SUCCESS) {
            return err;
        }
    }
    return FXM_SUCCESS;
}
FxChatError ClientConnection::sendMsg(const FxMessage *msg) {
    this->_doSend(msg);
    return FXM_SUCCESS;
}
