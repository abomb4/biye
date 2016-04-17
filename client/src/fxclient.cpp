#include "fxclient.h"
#include <iostream>

using namespace FxChat;

FxChatError FxClient::login(const QString *name, const QString *password) {
    FxChatError e = FxChatError::FXM_SUCCESS;
    FxConnection *c = FxConnection::getServerConnection();
    c->lock();
    FxMessage *msg = new (c->borrowFromPool(sizeof(FxMessage))) FxMessage();
    msg->fno(FxFunction::Login);
    FxMessageParam *param_tmp;
    // username
    {
        param_tmp = new (c->borrowFromPool(sizeof(FxMessageParam))) FxMessageParam();
        param_tmp->setName("name", 4);
        param_tmp->setVal(name->toStdString().c_str(), name->length());
        msg->addParam(param_tmp);
        param_tmp = nullptr;
    }
    // password
    {
        param_tmp = new (c->borrowFromPool(sizeof(FxMessageParam))) FxMessageParam();
        param_tmp->setName("password", 8);
        param_tmp->setVal(password->toStdString().c_str(), password->length());
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
        std::cout << recvMsg->toCharStr(c->borrowFromPool(recvMsg->needBufferSize()), recvMsg->needBufferSize()) << std::endl;
        c->unlock();
    } else {
        c->unlock();
    }
    return e;
}
