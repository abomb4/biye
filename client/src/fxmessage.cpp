#include "fxmessage.h"

#include <new>
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
    memset(newstr, 0, l);
    strncpy(newstr, this->name, this->l_name);
    return newstr;
}
const char *FxMessageParam::getVal(MemoryPool *pool) const {
    int l = this->l_val+ 1;
    char *newstr = pool->borrow(l);
    memset(newstr, 0, l);
    strncpy(newstr, this->val, this->l_val);
    return newstr;
}


// FxMessage::
FxMessage::FxMessage() {
    this->__bodylength = 0;
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
    this->__bodylength += addr->l_name + addr->l_val + 2;  // 2 is ':' and '\n'
}
void FxMessage::fno(uint16_t fno) { this->_fno = fno; }
uint16_t FxMessage::fno() const { return this->_fno; }

uint32_t FxMessage::bodyLength() const {
    return this->__bodylength;
}

void _append(char *&buffer_c, const char *src, int length) {
    if (length < 1)
        return;
    memcpy(buffer_c, src, length);
    buffer_c += length;
}

// NEED bodyLength() byte memory !!!
bool FxMessage::bodyStr(char *buffer, unsigned int length) {
    if (length < this->bodyLength())
        return false;

    FxMessageParam *cur = this->_body_list;
    while (cur != nullptr) {
        _append(buffer, cur->name, cur->l_name);
        _append(buffer, ":", 1);
        _append(buffer, cur->val, cur->l_val);
        _append(buffer, "\n", 1);
        cur = cur->_next;
    }
    return true;
}

void _make_header(char *&buffer, uint16_t len, uint16_t psize, uint16_t pno, uint16_t fno) {
    buffer[0] = (len >> 8) & 0xFF;
    buffer[1] = len & 0xFF;
    buffer[2] = (psize >> 8) & 0xFF;
    buffer[3] = psize & 0xFF;
    buffer[4] = (pno >> 8) & 0xFF;
    buffer[5] = pno & 0xFF;
    buffer[6] = (fno >> 8) & 0xFF;
    buffer[7] = fno & 0xFF;
    buffer += 8;
}

void _add_to_body(char **packages, int *plengths, char *&buffer_c, const char *src, int src_length, uint16_t pagesize, uint16_t &pageno, uint16_t fno, int full_len, int &current_len) {
    uint16_t msglen = 0; // first  2 byte
    if (full_len < FXMESSAGE_BLOCK_SIZE) {
        msglen = full_len;
    } else if (pagesize == pageno) {
        msglen = full_len % FXMESSAGE_BLOCK_SIZE;
        if (msglen == 0) msglen = FXMESSAGE_BLOCK_SIZE;
    } else {
        msglen = FXMESSAGE_BLOCK_SIZE;
    }

    // border
    if (current_len + src_length > FXMESSAGE_BLOCK_SIZE) {
        int x = FXMESSAGE_BLOCK_SIZE - current_len;
        _append(buffer_c, src, x);
        pageno += 1;
        current_len = 0;
        _add_to_body(packages, plengths, buffer_c, src + x, src_length - x, pagesize, pageno, fno, full_len, current_len);
    } else { // normal
        if (current_len == 0) { // make first header
            packages[pageno - 1] = buffer_c;
            _make_header(buffer_c, htons(msglen), htons(pagesize), htons(pageno), htons(fno));
            plengths[pageno - 1] = msglen + 8;
        }
        _append(buffer_c, src, src_length);
        current_len += src_length;
    }
}

int FxMessage::toPackages(char **&packages, int *&plengths, MemoryPool *pool) const {
    int body_length = this->__bodylength;
    uint16_t package_sum = (body_length - 1) / FXMESSAGE_BLOCK_SIZE + 1;
    if (package_sum > FXMESSAGE_BLOCK_MAX)
        return 0;
    packages = new (pool->borrow(sizeof(char*[package_sum]))) char*[package_sum];
    plengths = new (pool->borrow(sizeof(int[package_sum]))) int[package_sum];

    char *body = pool->borrow(body_length + package_sum * 8);
    char *current = body;

    FxMessageParam *cur = this->_body_list;
    int current_body_len = 0;
    uint16_t package_num = 1;
    while (cur != nullptr) {
        _add_to_body(packages, plengths, current, cur->name, cur->l_name, package_sum, package_num, this->_fno, body_length, current_body_len);
        _add_to_body(packages, plengths, current, ":", 1, package_sum, package_num, this->_fno, body_length, current_body_len);
        _add_to_body(packages, plengths, current, cur->val, cur->l_val, package_sum, package_num, this->_fno, body_length, current_body_len);
        _add_to_body(packages, plengths, current, "\n", 1, package_sum, package_num, this->_fno, body_length, current_body_len);
        cur = cur->_next;
    }
    return package_sum;
}
