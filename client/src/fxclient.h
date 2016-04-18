#ifndef FXCLIENT_H
#define FXCLIENT_H

#include <QString>
#include "fxmessage.h"
#include "fxconnection.h"

namespace FxChat {
namespace FxClient {

FxChatError login(const QString *name, const QString *password);

}
}

#endif // FXCLIENT_H
