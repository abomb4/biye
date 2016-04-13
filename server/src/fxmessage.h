#ifndef FXMESSAGE_H
#define FXMESSAGE_H

#include <cstdint>
#include "memorypool.h"

#define MAX_FXMESSAGE_SIZE 2048

namespace FxChat {

enum FxFunction : uint16_t {
    REPLAY = 0,
    Login = 1,
    GetUserList = 2,
    ChangePassword = 3,
    GetUserDetail = 4,
    SendMessage = 5
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
    short l_name;
    short l_val;

    FxMessageParam *_next;
};

class FxMessage {
public:
    FxMessage();
    ~FxMessage();

    // getter setter
    void fromUser(uint32_t uid);
    const uint32_t fromUser() const;
    void fno(uint16_t uid);
    const uint16_t fno() const;
    const FxMessageParam* paramList() const { return this->_body_list; }

    void addParam(FxMessageParam *addr);

    int needBufferSize();

    // NEED needBufferSize() byte memory !!!
    bool toCharStr(char *buffer, int length);

private:
    uint16_t _bodylength;
    uint32_t _from_user;
    uint16_t _fno;
    FxMessageParam *_body_list; // link list
    FxMessageParam *__list_current; // link list current

    // for toCharStr write buffer
    void _append(char *&buffer_c, const char *src, int length);
};
enum FxChatError : uint16_t {
    FXM_SUCCESS = 0,
    FXM_SOCKET_ERR = 1,
    FXM_MSG_TOO_SHORT = 2,
    FXM_PARSE_FAIL = 3,
    FXM_MSG_TOO_LONG = 4,
    FXM_UNKNOWN_FNO = 5,
    FXM_PAREMETER_CHECK_FAIL = 6,
};

}
#endif // FXMESSAGE_H
