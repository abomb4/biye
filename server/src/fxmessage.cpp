#include "fxmessage.h"

#include <cstdint>
#include <cstring>
#include <cstdio>
#include <arpa/inet.h>

using namespace FxChat;

// FxMessageParam::
FxMessageParam::FxMessageParam() {
    this->name = nullptr;
    this->val = nullptr;
    this->l_name = 0;
    this->l_val = 0;
    this->_next = nullptr;
}
FxMessageParam::~FxMessageParam() {
    if (this->_next != nullptr)
        this->_next->~FxMessageParam();
}

void FxMessageParam::setName(const char *addr, int length) {
    this->l_name = length;
    this->name = addr;
}
void FxMessageParam::setVal(const char *addr, int length) {
    this->l_val = length;
    this->val = addr;
}
const char *FxMessageParam::getName(MemoryPool *pool) const {
    int l = this->l_name + 1;
    char *newstr = pool->borrow(l);
    bzero(newstr, l);
    strncpy(newstr, this->name, this->l_name);
    return newstr;
}
const char *FxMessageParam::getVal(MemoryPool *pool) const {
    int l = this->l_val+ 1;
    char *newstr = pool->borrow(l);
    bzero(newstr, l);
    strncpy(newstr, this->val, this->l_val);
    return newstr;
}


// FxMessage::
FxMessage::FxMessage() {
    this->_bodylength = 0;
    this->_from_user = 0;
    this->_fno = 0;
    this->_body_list = nullptr;
    this->__list_current = nullptr;
}

FxMessage::~FxMessage() {
    if (this->_body_list != nullptr)
        this->_body_list->~FxMessageParam();
}

void FxMessage::addParam(FxMessageParam *addr) {
    if (this->_body_list == nullptr) {
        this->_body_list = addr;
        this->__list_current = this->_body_list;
    } else {
        this->__list_current->_next = addr;
        this->__list_current = this->__list_current->_next;
    }
    this->_bodylength += addr->l_name + addr->l_val + 2;  // 2 is ':' and '\n'
}
void FxMessage::fromUser(uint32_t uid) { this->_from_user = uid; }
const uint32_t FxMessage::fromUser() const { return this->_from_user; }
void FxMessage::fno(uint16_t fno) { this->_fno = fno; }
const uint16_t FxMessage::fno() const { return this->_fno; }

int FxMessage::needBufferSize() {
    return this->_bodylength + sizeof(_bodylength) + sizeof(_from_user) + sizeof(_fno);
}

// NEED needBufferSize() byte memory !!!
bool FxMessage::toCharStr(char *buffer, int length) {
    if (length < this->needBufferSize())
        return false;

    // convert from host byte order to network byte order
    uint16_t body_length_l = htons(this->_bodylength); // convert 1 total 3
    uint32_t from_user_l = htonl(this->_from_user); // convert 2 total 3
    uint16_t fno_l = htons(this->_fno); // convert 3 total 3
    buffer[0] = (body_length_l >> 8) & 0xFF;
    buffer[1] = body_length_l & 0xFF;
    buffer[2] = (from_user_l >> 24) & 0xFF;
    buffer[3] = (from_user_l >> 16) & 0xFF;
    buffer[4] = (from_user_l >> 8) & 0xFF;
    buffer[5] = from_user_l & 0xFF;
    buffer[6] = (fno_l >> 8) & 0xFF;
    buffer[7] = fno_l & 0xFF;

    char *buffer_c = buffer + 8;
    FxMessageParam *cur = this->_body_list;
    while (cur != nullptr) {
        this->_append(buffer_c, cur->name, cur->l_name);
        this->_append(buffer_c, ":", 1);
        this->_append(buffer_c, cur->val, cur->l_val);
        this->_append(buffer_c, "\n", 1);
        cur = cur->_next;
    }
    return true;
}
void FxMessage::_append(char *&buffer_c, const char *src, int length) {
    memcpy(buffer_c, src, length);
    buffer_c += length;
}
