#include "fxserver.h"
#include <new>
#include <cstdio>

#include <spdlog/spdlog.h>

#include "usermanager.h"
#include "usersession.h"
#include "fxcjson.h"

namespace FxChat {
namespace FxServer{

std::shared_ptr<spdlog::logger> __logger__ = nullptr;
std::shared_ptr<spdlog::logger> _logger() {
    if (__logger__ == nullptr)
        __logger__ = spdlog::get("FxServer");

    return __logger__;
}

FxChatError _login(FxMessage *&retmsg, const FxMessage *msg, ClientConnection *c);
FxChatError _get_user_list_full(FxMessage *&retmsg, const FxMessage *msg, ClientConnection *c);
FxChatError _get_user_list_diff(FxMessage *&retmsg, const FxMessage *msg, ClientConnection *c);
FxChatError _get_department_list_full(FxMessage *&retmsg, const FxMessage *msg, ClientConnection *c);
FxChatError _get_department_list_diff(FxMessage *&retmsg, const FxMessage *msg, ClientConnection *c);
FxChatError _get_online_users(FxMessage *&retmsg, const FxMessage *msg, ClientConnection *c);
FxChatError _get_user_detail(FxMessage *&retmsg, const FxMessage *msg, ClientConnection *c);
FxChatError _send_message(FxMessage *&retmsg, const FxMessage *msg, ClientConnection *c);

FxChatError doOperation(FxMessage *&retmsg, const FxMessage *msg, ClientConnection *c) {
    FxChatError (*_func)(FxMessage *&, const FxMessage *, ClientConnection *);
    bool unknown = false;
    switch (msg->fno()) {
    case FxFunction::FXF_Login:
        _func = _login; break;
    case FxFunction::FXF_GetUserListFull:
        _func = _get_user_list_full;break;
    case FxFunction::FXF_GetUserListDiff:
        _func = _get_user_list_diff;break;
    case FxFunction::FXF_GetDepartemntListFull:
        _func = _get_department_list_full;break;
    case FxFunction::FXF_GetDepartmentListDiff:
        _func = _get_department_list_diff;break;
    case FxFunction::FXF_GetOnlineUsers:
        _func = _get_online_users;break;
    case FxFunction::FXF_GetUserDetail:
        _func = _get_user_detail;break;
    case FxFunction::FXF_SendMessage:
        _func = _send_message;break;
    default:
        unknown = true;
    };

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
    // rec->fromUser(from_user);
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

///////////////////////////////////// Login
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
    _logger()->info("({}) do login", (void*)c);
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

    uint32_t userid;
    bool succ = UserManager::getInstance().login(pack->name, pack->password, userid);
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
    // deal with userid
    if (succ) {
        User user;
        user.id(userid);
        UserSession::createSession(c, user);
        param = new (pool->borrow(sizeof(FxMessageParam))) FxMessageParam();
        param->setName("userid", 6);
        char *uid = new (pool->borrow(sizeof(char) * 11)) char[11];
        sprintf(uid, "%d", userid);
        param->setVal(uid, strlen(uid));
        retmsg->addParam(param);
        param = nullptr;
    }
    _logger()->info("({}) do login finish", (void*)c);

    return FxChatError::FXM_SUCCESS;
}
// Login end

///////////////////////////////////// GetUserListFull
/// 本方法没有传入参数 ///
FxChatError _get_user_list_full(FxMessage *&retmsg, const FxMessage *msg, ClientConnection *c) {
    _logger()->info("({}) get full user list", (void*)c);
    User *list;
    int count = UserManager::getInstance().getFullList(list, c->pool());
    cJSON_set_pool(c->pool());
    cJSON *array = cJSON_CreateArray();
    for (int i = 0; i < count; i++) {
        cJSON *obj = cJSON_CreateObject();
        cJSON_AddNumberToObject(obj, "id", list[i].id());
        cJSON_AddStringToObject(obj, "name", list[i].name().c_str());
        cJSON_AddNumberToObject(obj, "department", list[i].department());
        cJSON_AddStringToObject(obj, "true_name", list[i].trueName().c_str());
        cJSON_AddItemToArray(array, obj);
    }
    char *liststr = cJSON_PrintUnformatted(array);
    retmsg = new (c->pool()->borrow(sizeof(FxMessage))) FxMessage();
    FxMessageParam *param;
    // deal with userlist
    {
        param = new (c->pool()->borrow(sizeof(FxMessageParam))) FxMessageParam();
        param->setName("userlist", 8);
        param->setVal(liststr, strlen(liststr));
        retmsg->addParam(param);
        param = nullptr;
    }

    _logger()->info("({}) get full user list finish", (void*)c);
    return FxChatError::FXM_SUCCESS;
}
// GetUserListFull end

///////////////////////////////////// GetUserListDiff
FxChatError _get_user_list_diff(FxMessage *&retmsg, const FxMessage *msg, ClientConnection *c) {

    return FxChatError::FXM_SUCCESS;
}
// GetUserListDiff end

///////////////////////////////////// GetDepartemntListFull
FxChatError _get_department_list_full(FxMessage *&retmsg, const FxMessage *msg, ClientConnection *c) {

    return FxChatError::FXM_SUCCESS;
}
// GetDepartemntListFull end

///////////////////////////////////// GetDepartemntListDiff
FxChatError _get_department_list_diff(FxMessage *&retmsg, const FxMessage *msg, ClientConnection *c) {

    return FxChatError::FXM_SUCCESS;
}
// GetDepartemntListDiff end

///////////////////////////////////// GetOnlineUsers
FxChatError _get_online_users(FxMessage *&retmsg, const FxMessage *msg, ClientConnection *c) {

    return FxChatError::FXM_SUCCESS;
}
// GetOnlineUsers end

///////////////////////////////////// GetUserDetail
FxChatError _get_user_detail(FxMessage *&retmsg, const FxMessage *msg, ClientConnection *c) {

    return FxChatError::FXM_SUCCESS;
}
// GetUserDetail end

///////////////////////////////////// SendMessage
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
FxChatError _send_message(FxMessage *&retmsg, const FxMessage *msg, ClientConnection *c) {
    _logger()->info("({}) send message", (void*)c);
    bool succ = true;
    MemoryPool *pool = c->pool();

    msg_pack *pack = new (pool->borrow(sizeof(msg_pack))) msg_pack;
    const FxMessageParam *param_list = msg->paramList();
    while (param_list != nullptr) {
        _set_to_msg_pack(pack, param_list->getName(pool), param_list->getVal(pool));
        param_list = param_list->_next;
    }
    // check param
    if (pack->userid == 0 || pack->toid == 0 || pack->body == nullptr || pack->userid == pack->toid)
        return FxChatError::FXM_PAREMETER_CHECK_FAIL;

    // check to_user_id exists
    if (!UserManager::getInstance().exists(pack->toid)) {
        // pack fail and send back, return
        return FXM_USER_NOT_EXIST;
    }
    // find connection of to_user_id
    UserSession *s = UserSession::getSessionByUid(pack->toid);
    if (s == nullptr) {
        // save to database
        _logger()->error("Need save to datbase, but not implements.");
    } else {
        // send to this user
        s->connection()->lock();
        s->connection()->sendMsg(msg);
        s->connection()->pool()->clear();
        s->connection()->unlock();
    }

    retmsg = new (pool->borrow(sizeof(FxMessage))) FxMessage();
    FxMessageParam *param;
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
    _logger()->info("({}) send message finish", (void*)c);
    return FxChatError::FXM_SUCCESS;
}
// SendMessage end

}
}
