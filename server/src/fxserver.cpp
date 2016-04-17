#include "fxserver.h"
#include <new>
#include <cstdio>

#include <spdlog/spdlog.h>

#include "usermanager.h"

namespace FxChat {
namespace FxServer{

std::shared_ptr<spdlog::logger> __logger__ = nullptr;
std::shared_ptr<spdlog::logger> _logger() {
    if (__logger__ == nullptr)
        __logger__ = spdlog::get("FxServer");

    return __logger__;
}

FxChatError _login(FxMessage *&retmsg, const FxMessage *msg, ClientConnection *c);
FxChatError _get_user_list(FxMessage *&retmsg, const FxMessage *msg, ClientConnection *c);
FxChatError _change_password(FxMessage *&retmsg, const FxMessage *msg, ClientConnection *c);
FxChatError _get_user_detail(FxMessage *&retmsg, const FxMessage *msg, ClientConnection *c);
FxChatError _send_msg(FxMessage *&retmsg, const FxMessage *msg, ClientConnection *c);

FxChatError doOperation(FxMessage *&retmsg, const FxMessage *msg, ClientConnection *c) {
    FxChatError (*_func)(FxMessage *&, const FxMessage *, ClientConnection *);
    bool unknown = false;
    switch (msg->fno()) {
    case FxFunction::Login:
        _func = _login; break;

    case FxFunction::GetUserList:
        _func = _get_user_list; break;

    case FxFunction::ChangePassword:
        _func = _change_password; break;

    case FxFunction::GetUserDetail:
        _func = _get_user_detail; break;

    case FxFunction::SendMessage:
        _func = _send_msg; break;

    default:
        unknown = true;
    }

    if (unknown) {
        _logger()->warn("Unknown function no {}.", msg->fno());
        return FxChatError::FXM_UNKNOWN_FNO;
    }

    return _func(retmsg, msg, c);
}

void _errcode_to_char4(FxChatError e, char *buffer) {
    char msg[5];
    sprintf(msg, "%04d", e);
    strncpy(buffer, msg, 4);
}

void makeFailureMsg(FxMessage *&retmsg, FxChatError e, MemoryPool *pool, uint32_t from_user) {
    FxMessage *rec = new (pool->borrow(sizeof(FxMessage))) FxMessage;
    rec->fromUser(from_user);
    FxMessageParam *fmp_tmp = nullptr;
    char *c_tmp = nullptr;
    // deal with fno
    {
        fmp_tmp = new (pool->borrow(sizeof(FxMessageParam))) FxMessageParam();
        // name
        c_tmp = new (pool->borrow(3)) char(3);
        strncpy(c_tmp, "fno", 3);
        fmp_tmp->setName(c_tmp, 3);
        c_tmp = nullptr; // name end

        // val
        c_tmp = new (pool->borrow(4)) char(4);
        strncpy(c_tmp, "fail", 4);
        fmp_tmp->setVal(c_tmp, 4);
        c_tmp = nullptr; // val end

        // set fmp_tmp to FxMessage
        rec->addParam(fmp_tmp);
        fmp_tmp = nullptr;
    }
    // deal with reason
    {
        fmp_tmp = new (pool->borrow(sizeof(FxMessageParam))) FxMessageParam;
        // name
        c_tmp = new (pool->borrow(6)) char(6);
        strncpy(c_tmp, "reason", 6);
        fmp_tmp->setName(c_tmp, 6);
        c_tmp = nullptr; // name end

        // val
        c_tmp = new (pool->borrow(4)) char(4);
        _errcode_to_char4(e, c_tmp);
        fmp_tmp->setVal(c_tmp, 4);
        c_tmp = nullptr; // val end

        // set fmp_tmp to FxMessage
        rec->addParam(fmp_tmp);
        fmp_tmp = nullptr;
    }
    retmsg = rec;
}

typedef struct _s_login_pack {
    const char *name;
    const char *password;
} login_pack;
void _set_to_login_pack(login_pack *&pack, const char *name, const char *val) {
    if (strcmp(name, "name") == 0)
        pack->name = val;
    else if (strcmp(name, "password") == 0)
        pack->password = val;
}

FxChatError _login(FxMessage *&retmsg, const FxMessage *msg, ClientConnection *c) {
    _logger()->debug("({}) do login", (void*)c);
    MemoryPool *pool = c->pool();

    login_pack *pack = new (pool->borrow(sizeof(login_pack))) login_pack;
    const FxMessageParam *param_list = msg->paramList();
    while (param_list != nullptr) {
        _set_to_login_pack(pack, param_list->getName(pool), param_list->getVal(pool));
        param_list = param_list->_next;
    }
    // check param
    if (pack->name == nullptr || pack->password == nullptr)
        return FxChatError::FXM_PAREMETER_CHECK_FAIL;

    bool succ = UserManager::getInstance().login(pack->name, pack->password);
    retmsg = new (pool->borrow(sizeof(FxMessage))) FxMessage();
    FxMessageParam *param;
    // deal with name
    {
        param = new (pool->borrow(sizeof(FxMessageParam))) FxMessageParam();
        param->setName("name", 4);
        param->setVal(pack->name, strlen(pack->name));
        retmsg->addParam(param);
        param = nullptr;
    }
    // deal with stat
    {
        param = new (pool->borrow(sizeof(FxMessageParam))) FxMessageParam();
        param->setName("stat", 4);
        if (succ) { // login success
            param->setVal("succ", 4);
        } else { // login fail
            param->setVal("fail", 4);
        }
        retmsg->addParam(param);
        param = nullptr;
    }

    return FxChatError::FXM_SUCCESS;
}

FxChatError _get_user_list(FxMessage *&retmsg, const FxMessage *msg, ClientConnection *c) {

    return FxChatError::FXM_SUCCESS;
}

FxChatError _change_password(FxMessage *&retmsg, const FxMessage *msg, ClientConnection *c) {

    return FxChatError::FXM_SUCCESS;
}

FxChatError _get_user_detail(FxMessage *&retmsg, const FxMessage *msg, ClientConnection *c) {

    return FxChatError::FXM_SUCCESS;
}

FxChatError _send_msg(FxMessage *&retmsg, const FxMessage *msg, ClientConnection *c) {

    return FxChatError::FXM_SUCCESS;
}


}
}
