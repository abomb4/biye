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

using std::cout;
using std::endl;
using std::cerr;
using std::mutex;

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
    this->_listener_t.detach();
    close(this->_sockfd);
    // CANNOT DELETE BECAUSE OF BLOCKEDMEMORYPOOL
    LinearMemoryPool *_free = (LinearMemoryPool *)this->_pool;
    _free->~LinearMemoryPool();
    // delete this->_pool;

    // free
    (*this->_restore)((char *)this);
}

void ClientConnection::setRestoreFunc(void (*func)(char *)) {
    this->_restore = *func;
}

void ClientConnection::startListener() {
    this->_listener_t = std::thread([=] { _doListen(); });
}

void ClientConnection::_doListen() {
    char *buffer;
    bool _breakIt = false;
    while(true) {
        buffer = this->_pool->borrow(8);
        bzero(buffer, 8);
        // recieve msg head, //MSG_PEEK mode
        int n = recv(this->_sockfd, buffer, 8, 0);
        this->_send_mutex.lock(); // LOCK
        if (n < 0) {
            ClientConnection::_logger->error("({}) ERROR reading from socket", (void*)this);
            break;
        } else if (n == 0) {
            ClientConnection::_logger->info("({}) client closes connection", (void*)this);
            break;
        }
        FxMessage *returnMsg = nullptr;
        // less than 8Byte message
        if (n < 8) {
            FxServer::makeFailureMsg(returnMsg, FxChatError::FXM_MSG_TOO_SHORT, this->_pool, 0);
        } else { // normal
            FxMessage *recieveMsg = nullptr;
            FxChatError err = this->_doParse(buffer, recieveMsg);
            if (err != FxChatError::FXM_SUCCESS) { // wrong
                FxServer::makeFailureMsg(returnMsg, err, this->_pool, 0);
            } else {
                // LETS DO OPERATION!
                FxChatError err = FxServer::doOperation(returnMsg, recieveMsg, this);
                if (err != FxChatError::FXM_SUCCESS) {
                    FxServer::makeFailureMsg(returnMsg, err, this->_pool);
                    if (err == FxChatError::FXM_MSG_TOO_LONG || err == FxChatError::FXM_PARSE_FAIL)
                        _breakIt = true;
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

FxChatError ClientConnection::_doParse(const char *buffer_8, FxMessage *&msg) {
    // parse body length and from_user first
    uint16_t msg_body_length = 0;
    msg_body_length += buffer_8[0] << 8;
    msg_body_length += buffer_8[1];
    msg_body_length = ntohs(msg_body_length);
    if (msg_body_length > MAX_FXMESSAGE_SIZE - 8) { // message body too long
        return FxChatError::FXM_MSG_TOO_LONG;
    }
    msg = new (this->_pool->borrow(sizeof(FxMessage))) FxMessage();

    uint32_t from_user_n = 0;
    from_user_n += buffer_8[2] << 24;
    from_user_n += buffer_8[3] << 16;
    from_user_n += buffer_8[4] << 8;
    from_user_n += buffer_8[5];
    msg->fromUser(ntohl(from_user_n));

    uint16_t fno = 0;
    fno += buffer_8[6] << 8;
    fno += buffer_8[7];
    msg->fno(ntohs(fno));

    // do read
    char *msg_body_buff = this->_pool->borrow(msg_body_length);
    int n = 0, need = msg_body_length;
    while(need > 0) {
        n = recv(this->_sockfd, msg_body_buff + msg_body_length - need, msg_body_length, 0);
        if (n < 0) {
            ClientConnection::_logger->error("({}) ERROR reading from socket in _doParse()", (void*)this);
            return FxChatError::FXM_SOCKET_ERR;
        } else if (n == 0) {
            ClientConnection::_logger->info("({}) client closes connection in _doParse()", (void*)this);
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
        int bodylen = msg_body_length - (colon_pos + 1 - msg_body_buff);
        p = new (this->_pool->borrow(sizeof(FxMessageParam))) FxMessageParam;
        p->setName(line_cur, (int)(colon_pos - line_cur - 1)); // minus ':'
        p->setVal(colon_pos + 1, bodylen);
        msg->addParam(p);
    }
    return FxChatError::FXM_SUCCESS;
}

void ClientConnection::_doSend(FxMessage *x) {
    int buffer_length = x->needBufferSize();
    char *buffer = this->_pool->borrow(buffer_length);
    if (!x->toCharStr(buffer, buffer_length)) {
        // ERROR
        _logger->error("FxMessage To CharStr FAILED!!");
        int n = write(this->_sockfd, "failed to build msg", 19);
        if (n < 0)
            ClientConnection::_logger->error("({}) ERROR writing to socket", (void*)this);
    } else {
        int n = 0;
        n = write(this->_sockfd, buffer, buffer_length);
        if (n < 0)
            ClientConnection::_logger->error("({}) ERROR writing to socket", (void*)this);
    }
}
