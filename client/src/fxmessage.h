#ifndef FXMESSAGE_H
#define FXMESSAGE_H

#include <cstdint>
#include "memorypool.h"

#define FXMESSAGE_BLOCK_SIZE 32760
#define FXMESSAGE_BLOCK_MAX  128

namespace FxChat {

enum FxFunction : uint16_t {
    FXF_REPLAY = 0,
    FXF_Login = 1,
    FXF_GetUserListFull = 2,
    FXF_GetUserListDiff = 3,
    FXF_GetDepartemntListFull = 4,
    FXF_GetDepartmentListDiff = 5,
    FXF_GetOnlineUsers = 6,
    FXF_GetUserDetail = 7,
    FXF_SendMessage = 8
};

class FxMessageParam {
public:
    FxMessageParam();
    ~FxMessageParam();

    void setName(const char *addr, int length);
    void setVal(const char *addr, int length);
    // return c str end with '\0'
    const char *getName(MemoryPool *pool) const;
    const char *getVal(MemoryPool *pool) const;

    const char *name;
    const char *val;
    uint32_t l_name;
    uint32_t l_val;

    FxMessageParam *_next;
};

class FxMessage {
public:
    FxMessage();
    ~FxMessage();

    // getter setter
    void fno(uint16_t uid);
    uint16_t fno() const;
    const FxMessageParam* paramList() const { return this->_body_list; }

    void addParam(FxMessageParam *addr);

    uint32_t bodyLength() const;

    // NEED bodyLength() byte memory !!!
    bool bodyStr(char *buffer, unsigned int length);

    // return pack sum
    int toPackages(char **&buffer, int *&plengths, MemoryPool *pool) const;

private:
    uint32_t __bodylength;
    uint16_t _fno;
    FxMessageParam *_body_list; // link list
    FxMessageParam *__list_current; // link list current
};
enum FxChatError : uint16_t {
    FXM_SUCCESS                 = 0,
    FXM_FAIL                    = 1,
    FXM_SOCKET_ERR              = 2,
    FXM_MSG_TOO_SHORT           = 3,
    FXM_PARSE_FAIL              = 4,
    FXM_MSG_TOO_LONG            = 5,
    FXM_UNKNOWN_FNO             = 6,
    FXM_PAREMETER_CHECK_FAIL    = 7,
    FXM_TIME_OUT                = 8,
    FXM_NO_RESPONSE_RECIEVED    = 9,
};

}
#endif // FXMESSAGE_H
