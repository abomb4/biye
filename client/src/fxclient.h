#ifndef FXCLIENT_H
#define FXCLIENT_H

#include <QString>
#include "fxmessage.h"
#include "fxconnection.h"

namespace FxChat {
namespace FxClient {

extern uint32_t USER_ID;

FxChatError login(const QString *name, const QString *password);

FxChatError getFullUserList();
FxChatError getDiffUserList(const QString *timestamp);

FxChatError sendMsg(const uint32_t to_user_id);

}
}

#endif // FXCLIENT_H
