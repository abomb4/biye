#include "fxclient.h"
#include <iostream>
#include <cstdio>

#include <QApplication>

#include "clientdb.h"
#include "fxconnection.h"

using namespace FxChat;

FxClient *FxClient::_instance = nullptr;
FxClient *FxClient::getInstance() {
    if (_instance == nullptr) {
        _instance = new FxClient();
    }
    return _instance;
}
FxClient::FxClient() {
    FxConnection *c = FxConnection::getServerConnection();
}
FxClient::~FxClient() {
}

void FxClient::quitApplication() {
    delete _instance;
    _instance = nullptr;
    stopHeartBeat();
    stopListenMsg();
    delete FxConnection::getServerConnection();
    ClientDB::destroydb();
    qApp->quit();
    qApp->exit();
}

uint32_t FxClient::USER_ID;

char *_pooled_str(const char *source, int length, LinearMemoryPool *pool) {
    char *r = new (pool->borrow(length)) char[length];
    strncpy(r, source, length);
    return r;
}

class loginResult{
public:
    loginResult() { this->name = nullptr; this->stat = nullptr; this->userid = nullptr; }
    const char *name;
    const char *stat;
    const char *userid;
};
void _set_to_login_result(loginResult *&result, const char *name, const char *val) {
    if (strcmp(name, "name") == 0)
        result->name = val;
    else if (strcmp(name, "stat") == 0)
        result->stat = val;
    else if (strcmp(name, "userid") == 0)
        result->userid = val;
}

FxChatError FxClient::login(const QString *name, const QString *password) {
    qDebug() << "login()";
    bool logined = false;
    FxChatError e = FxChatError::FXM_SUCCESS;
    FxConnection *c = FxConnection::getServerConnection();
    c->lock();
    FxMessage *msg = new (c->borrowFromPool(sizeof(FxMessage))) FxMessage();
    msg->fno(FxFunction::FXF_Login);
    FxMessageParam *param_tmp;
    // username
    {
        param_tmp = new (c->borrowFromPool(sizeof(FxMessageParam))) FxMessageParam();
        param_tmp->setName(_pooled_str("name", 4, c->getPool()), 4);
        param_tmp->setVal(
                    _pooled_str(name->toStdString().c_str(), name->length(), c->getPool()),
                    name->length());
        msg->addParam(param_tmp);
        param_tmp = nullptr;
    }
    // password
    {
        param_tmp = new (c->borrowFromPool(sizeof(FxMessageParam))) FxMessageParam();
        param_tmp->setName(_pooled_str("password", 8, c->getPool()), 8);
        param_tmp->setVal(
                    _pooled_str(password->toStdString().c_str(), password->length(), c->getPool()),
                    password->length());
        msg->addParam(param_tmp);
        param_tmp = nullptr;
    }
    e = c->send(msg);
    if (e != FxChatError::FXM_SUCCESS) { // send fail
        c->unlock();
        return e;
    }
    FxMessage *recvMsg;
    e = c->receive(recvMsg);
    if (e == FxChatError::FXM_SUCCESS) {
        // 验证登录有没有成功
        loginResult *result = new (c->getPool()->borrow(sizeof(loginResult))) loginResult;
        // build loginResult struct
        const FxMessageParam *param_list = recvMsg->paramList();
        while (param_list != nullptr) {
            _set_to_login_result(result, param_list->getName(c->getPool()), param_list->getVal(c->getPool()));
            param_list = param_list->_next;
        }
        if (result->stat != nullptr && strcmp(result->stat, "succ") == 0) { // success
            logined = true;
            USER_ID = atoi(result->userid);
            e = FxChatError::FXM_SUCCESS;
        } else { // fail
            e = FxChatError::FXM_FAIL;
        }
    } else {

    }
    c->unlock();
    if (!logined)
        delete c;
    else
        c->clearPool();
    qDebug() << "login() finished";
    return e;
}

FxChatError FxClient::getUserList(QMap<uint32_t, User> *&v) {
    qDebug() << "getUserList()";
    QDateTime now = QDateTime::currentDateTime();
    FxChatError e = FxChatError::FXM_SUCCESS;
    FxConnection *c = FxConnection::getServerConnection();
    c->lock();
    // TODO 1 getLastUserUpdateTime
    QString last_time = ClientDB::getLastUserUpdateTime();
    //      2 if '' get from server, else get diff list from server
    if (last_time.isEmpty()) {
        // getall
        FxMessage *msg = new (c->borrowFromPool(sizeof(FxMessage))) FxMessage();
        msg->fno(FxFunction::FXF_GetUserListFull);
        FxMessageParam *param_tmp;
        // userid
        {
            param_tmp = new (c->borrowFromPool(sizeof(FxMessageParam))) FxMessageParam();
            param_tmp->setName(_pooled_str("userid", 6, c->getPool()), 6);
            char *idc = new (c->borrowFromPool(sizeof(char[11]))) char[11];
            sprintf(idc, "%d", USER_ID);
            param_tmp->setVal(idc, strlen(idc));
            msg->addParam(param_tmp);
            param_tmp = nullptr;
        }
        e = c->send(msg);
        if (e != FxChatError::FXM_SUCCESS) { // send fail
            qDebug() << "SEND FAIL!" << e;
            c->clearPool();
            c->unlock();
            return e;
        }
        FxMessage *recvMsg;
        e = c->receive(recvMsg);
        if (e == FxChatError::FXM_SUCCESS) {
            // convert userlist json to vector
            const FxMessageParam *p = recvMsg->paramList();
            if (strcmp(p->getName(c->getPool()), "userlist") != 0) { // error

            } else {
                v = new QMap<uint32_t, User>();
                QVector<User> *v2 = new QVector<User>();
                const char *userlist = p->getVal(c->getPool());
                QJsonDocument json = QJsonDocument::fromJson(userlist);
                QJsonArray arr = json.array();
                QJsonObject obj;
                for (int i = 0; i < arr.size(); i++) {
                    obj = arr.at(i).toObject();
                    User u;
                    u.id(obj["id"].toInt());
                    u.name(obj["name"].toString());
                    u.trueName(obj["true_name"].toString());
                    u.department(obj["department"].toInt());
                    v->insert(u.id(), u);
                    v2->append(u);
                }
                ClientDB::addUsers(v2);
            }
        } else {
            qDebug() << "RECEIVE FAIL!" << e;
            c->clearPool();
            c->unlock();
            return e;
        }
        last_time = now.toString("yyyyMMddhhmmss");
        ClientDB::setLastUserUpdateTime(last_time);
    } else {
        //      3 if diff list not empty, update db
        //      4 get from db
        v = new QMap<uint32_t, User>();
        QVector<User> *v2 = ClientDB::getUsers();
        QVector<User>::iterator i;
        for (i = v2->begin(); i != v2->end(); i++) {
            v->insert(i->id(), *i);
        }
        delete v2;
    }
    c->clearPool();
    c->unlock();
    qDebug() << "getUserList() finished";
    return FXM_SUCCESS;
}
FxChatError FxClient::getOnline(QVector<uint32_t> *&v) {
    // getall
    FxChatError e = FxChatError::FXM_SUCCESS;
    FxConnection *c = FxConnection::getServerConnection();
    FxMessage *msg = new (c->borrowFromPool(sizeof(FxMessage))) FxMessage();
    msg->fno(FxFunction::FXF_GetOnlineUsers);
    FxMessageParam *param_tmp;
    // userid
    {
        param_tmp = new (c->borrowFromPool(sizeof(FxMessageParam))) FxMessageParam();
        param_tmp->setName(_pooled_str("userid", 6, c->getPool()), 6);
        char *idc = new (c->borrowFromPool(sizeof(char[11]))) char[11];
        sprintf(idc, "%d", USER_ID);
        param_tmp->setVal(idc, strlen(idc));
        msg->addParam(param_tmp);
        param_tmp = nullptr;
    }
    e = c->send(msg);
    if (e != FxChatError::FXM_SUCCESS) { // send fail
        qDebug() << "SEND FAIL!" << e;
        c->clearPool();
        c->unlock();
        return e;
    }
    FxMessage *recvMsg;
    e = c->receive(recvMsg);
    if (e == FxChatError::FXM_SUCCESS) {
        // convert userlist json to vector
        const FxMessageParam *p = recvMsg->paramList();
        if (strcmp(p->getName(c->getPool()), "userlist") != 0) { // error

        } else {
            v = new QVector<uint32_t>();
            const char *userlist = p->getVal(c->getPool());
            QJsonDocument json = QJsonDocument::fromJson(userlist);
            QJsonArray arr = json.array();
            for (int i = 0; i < arr.size(); i++) {
                v->append(arr.at(i).toInt());
            }
        }
    } else {
        qDebug() << "RECEIVE FAIL!" << e;
        c->clearPool();
        c->unlock();
        return e;
    }
    c->clearPool();
    c->unlock();
    return e;
}

User *FxClient::getUserInfo(const uint32_t userid) {
    return ClientDB::getUserById(userid);
}

FxChatError FxClient::getDepartmentList(QVector<Department> *&v) {

}

FxChatError FxClient::getRecent(QVector<uint32_t> *&v) {
    v = ClientDB::getRecent(USER_ID);
    return FXM_SUCCESS;
}

FxChatError FxClient::addRecent(const uint32_t target_id) {
    if (ClientDB::addRecent(USER_ID, target_id))
        return FXM_SUCCESS;
    else return FXM_FAIL;
}

FxChatError FxClient::getChatLog(const uint32_t target_id) {

}

// current version send to server
FxChatError FxClient::sendMsg(const uint32_t to_user_id, const QString &messagebody) {
    qDebug() << "sendMsg()";
    FxChatError e = FxChatError::FXM_SUCCESS;
    ChatLog l;
    l.source_id(USER_ID);
    l.target_id(to_user_id);
    l.type(ChatLog::SEND);
    l.msg(messagebody);
    if (!ClientDB::addLog(l)) {
        qDebug() << "call ClientDB::addLog(l) fail!";
    }
    FxConnection *c = FxConnection::getServerConnection();
    c->lock();
    FxMessage *msg = new (c->borrowFromPool(sizeof(FxMessage))) FxMessage();
    msg->fno(FxFunction::FXF_SendMessage);
    FxMessageParam *param_tmp;
    // userid
    {
        param_tmp = new (c->borrowFromPool(sizeof(FxMessageParam))) FxMessageParam();
        param_tmp->setName(_pooled_str("userid", 6, c->getPool()), 6);
        char *idc = new (c->borrowFromPool(sizeof(char[11]))) char[11];
        sprintf(idc, "%d", USER_ID);
        param_tmp->setVal(idc, strlen(idc));
        msg->addParam(param_tmp);
        param_tmp = nullptr;
    }
    // toid
    {
        param_tmp = new (c->borrowFromPool(sizeof(FxMessageParam))) FxMessageParam();
        param_tmp->setName(_pooled_str("toid", 4, c->getPool()), 4);
        char *idc = new (c->borrowFromPool(sizeof(char[11]))) char[11];
        sprintf(idc, "%d", to_user_id);
        param_tmp->setVal(idc, strlen(idc));
        msg->addParam(param_tmp);
        param_tmp = nullptr;
    }
    // body
    {
        param_tmp = new (c->borrowFromPool(sizeof(FxMessageParam))) FxMessageParam();
        param_tmp->setName(_pooled_str("body", 4, c->getPool()), 4);
        QByteArray a = messagebody.toUtf8();
        char *val = c->getPool()->borrow(a.length());
        memcpy(val, a, a.length());
        param_tmp->setVal(val, a.length());
        msg->addParam(param_tmp);
        param_tmp = nullptr;
    }
    e = c->send(msg);
    if (e != FxChatError::FXM_SUCCESS) { // send fail
        qDebug() << "sendMsg() fail";
        c->unlock();
        return e;
    }
    FxMessage *recieveMsg;
    e = c->receive(recieveMsg);
    if (e != FXM_SUCCESS) {
    } else {
        // judge stat
        const FxMessageParam *param_list = msg->paramList();
        while (param_list != nullptr) {

            param_list = param_list->_next;
        }
    }

    c->unlock();

    qDebug() << "sendMsg() finished";
    return e;
}


// slots
// no slots

ListenThread *_lt = nullptr;
bool FxClient::startListenMsg() {
    if (_lt == nullptr) {
        _lt = new ListenThread(FxClient::getInstance());
        _lt->start();
        return true;
    } else
        return false;

}

bool FxClient::stopListenMsg() {
    if (_lt == nullptr)
        return false;
    else {
        _lt->terminate();
        _lt->wait(1000);
        delete _lt;
        _lt = nullptr;
        return true;
    }
}

HeartBeatThread *_ht = nullptr;
bool FxClient::startHeartBeat() {
    if (_ht == nullptr) {
        _ht = new HeartBeatThread();
        _ht->start();
        return true;
    } else
        return false;
}

bool FxClient::stopHeartBeat() {
    if (_ht == nullptr)
        return false;
    else {
        _ht->terminate();
        _lt->wait(1000);
        delete _ht;
        _ht = nullptr;
        return true;
    }
}

// threads
void HeartBeatThread::run() {

}

typedef struct _s_msg_pack {
    uint32_t userid;
    uint32_t toid;
    const char *body;
} msg_pack;
void _set_to_msg_pack(msg_pack *&pack, const char *name, const char *val) {
    if (strcmp(name, "userid") == 0)
        pack->userid = strtoul(val, 0, 0);
    else if (strcmp(name, "toid") == 0)
        pack->toid = atoi(val);
    else if (strcmp(name, "body") == 0)
        pack->body = val;
}
void ListenThread::run() {
    FxConnection *sc = FxConnection::getServerConnection();
    while(true) {
        // QThread::sleep(1);
        sc->waitForReadyRead(-1);
        if (sc->operating()) {
            QThread::sleep(1);
            continue;
        }
        if (sc->haveMsg()) {
            qDebug() << "ListenThread recieved.";
            if (sc->operating())
                continue;
            sc->lock();
            qDebug() << "ListenThread Do.";
            // DO IT
            FxMessage *msg = nullptr;
            FxChatError err = sc->receive(msg);
            if (err != FXM_SUCCESS) {
                qDebug() << "ListenThread recieve fail!";
                return;
            }

            switch(msg->fno()) {
            case FXF_SendMessage: {
                msg_pack *pack = new (sc->getPool()->borrow(sizeof(msg_pack))) msg_pack;
                const FxMessageParam *param_list = msg->paramList();
                while (param_list != nullptr) {
                    _set_to_msg_pack(pack, param_list->getName(sc->getPool()), param_list->getVal(sc->getPool()));
                    param_list = param_list->_next;
                }
                QString msgbody = QString(pack->body);
                { // chatlog
                    ChatLog l;
                    l.source_id(FxClient::USER_ID);
                    l.target_id(pack->userid);
                    l.type(ChatLog::RECIEVE);
                    l.msg(msgbody);
                    if (!ClientDB::addLog(l)) {
                        qDebug() << "CALL ClientDB::addLog(l) fail!";
                    }
                }
                this->_client->receiveMsg(pack->userid, pack->toid, msgbody);
                break;
            }
            case FXF_ToOnline: {
                const FxMessageParam *idparam = msg->paramList(); // this must be userid
                if (strcmp(idparam->getName(sc->getPool()), "userid") != 0) {
                    qDebug() << "Recieve FXF_ToOnline but no userid at first!!!!!";
                } else {
                    this->_client->online(atoi(idparam->getVal(sc->getPool())));
                }
                break;
            }
            case FXF_ToOffline: {
                const FxMessageParam *idparam = msg->paramList(); // this must be userid
                if (strcmp(idparam->getName(sc->getPool()), "userid") != 0) {
                    qDebug() << "Recieve FXF_ToOnline but no userid at first!!!!!";
                } else {
                    this->_client->offline(atoi(idparam->getVal(sc->getPool())));
                }
                break;
            }
            default:
                qDebug() << "NOT NECESSARY MESSAGE!";break;
            }
            sc->clearPool();
            sc->unlock();
            qDebug() << "ListenThread Done.";
        }
    }
}
