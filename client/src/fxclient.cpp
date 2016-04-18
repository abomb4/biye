#include "fxclient.h"
#include <iostream>

using namespace FxChat;

char *_pooled_str(const char *source, int length, LinearMemoryPool *pool) {
    char *r = new (pool->borrow(length)) char[length];
    strncpy(r, source, length);
    return r;
}

class loginResult{
public:
    loginResult() { this->name = nullptr; this->stat = nullptr; }
    const char *name;
    const char *stat;
};
void _set_to_login_result(loginResult *&result, const char *name, const char *val) {
    if (strcmp(name, "name") == 0)
        result->name = val;
    else if (strcmp(name, "stat") == 0)
        result->stat = val;
}

FxChatError FxClient::login(const QString *name, const QString *password) {
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
            e = FxChatError::FXM_SUCCESS;
        } else { // fail
            e = FxChatError::FXM_FAIL;
        }
    } else {

    }
    c->unlock();
    if (!logined)
        delete c;
    return e;
}
