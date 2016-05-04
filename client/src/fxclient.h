#ifndef FXCLIENT_H
#define FXCLIENT_H

#include <QString>
#include "fxmessage.h"
#include "fxconnection.h"
#include "structs/user.h"
#include "structs/department.h"
#include "structs/chatlog.h"

namespace FxChat {
namespace FxClient {

extern uint32_t USER_ID;

FxChatError login(const QString *name, const QString *password);

// update db from server, return from db
FxChatError getUserList(QVector<User> *&v);

// return from db
User *getUserInfo(const uint32_t userid = USER_ID);

// update db from server, return from db
FxChatError getDepartmentList(QVector<Department> *&v);

// get from local db
FxChatError getRecent(QVector<uint32_t> *&v);

// write to local db
FxChatError addRecent(const uint32_t target_id);

FxChatError getChatLog(const uint32_t target_id);

FxChatError sendMsg(const uint32_t to_user_id, const QString *msg);

}
}

#endif // FXCLIENT_H
