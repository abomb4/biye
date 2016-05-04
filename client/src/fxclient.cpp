#include "fxclient.h"
#include <iostream>
#include <cstdio>

#include "clientdb.h"

using namespace FxChat;
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
    e = c->recieve(recvMsg);
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

FxChatError FxClient::getUserList(QVector<User> *&v) {
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
        e = c->recieve(recvMsg);
        if (e == FxChatError::FXM_SUCCESS) {
            // convert userlist json to vector
            const FxMessageParam *p = recvMsg->paramList();
            if (strcmp(p->getName(c->getPool()), "userlist") != 0) { // error

            } else {
                v = new QVector<User>();
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
                    v->append(u);
                }
                ClientDB::addUsers(v);
            }
        } else {
            qDebug() << "RECIEVE FAIL!" << e;
            c->clearPool();
            c->unlock();
            return e;
        }
        c->clearPool();
        c->unlock();
        last_time = now.toString("yyyyMMddhhmmss");
        ClientDB::setLastUserUpdateTime(last_time);
    } else {
        //      3 if diff list not empty, update db
        //      4 get from db
        v = ClientDB::getUsers();
    }
    qDebug() << "getUserList() finished";
    return FXM_SUCCESS;
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

FxChatError FxClient::sendMsg(const uint32_t to_user_id, const QString *msg) {

}
