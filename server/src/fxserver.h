#ifndef FXSERVER_H
#define FXSERVER_H

#include "fxmessage.h"
#include "memorypool.h"
#include "clientconnection.h"

namespace FxChat {
namespace FxServer{

///
/// \brief All request's entrance. LET'S DO SOMETHING!
///
/// \param retmsg
///         Don't new it before call this function.
/// \param msg
///         Message recieved from client
/// \param pool
///         Memory pool, avoid frequency new/delet
/// \return
///         FxChatError::FMX_SUCCESS is success, other is fail.
///         if fail, retmsg is nullptr.
///
FxChatError doOperation(FxMessage *&retmsg, const FxMessage *msg, ClientConnection *c);

///
/// \brief Make failure message
/// \param retmsg
///         Don't new it before call this function.
/// \param e
///         FxChat::FxChatError e
/// \param pool
///         Memory pool, avoid frequency new/delet
///
void makeFailureMsg(FxMessage *&retmsg, FxChatError e, MemoryPool *pool, uint32_t from_user = 0);

}
}

#endif // FXSERVER_H
