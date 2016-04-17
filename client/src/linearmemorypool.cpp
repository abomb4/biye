#include <iostream>
using std::cout;
using std::endl;

#include "linearmemorypool.h"

LinearMemoryPool::LinearMemoryPool(int size) {

    if (size < LINEARMEMORYPOOL_MIN_SIZE)
        size = LINEARMEMORYPOOL_MIN_SIZE;
    this->_size = size;
    this->_pool = new char[size + sizeof(LinearMemoryPool)];
    this->_current = this->_pool;
    this->_next = nullptr;
}

LinearMemoryPool::~LinearMemoryPool() {
    this->clear();
    delete[] this->_pool;
}

char* LinearMemoryPool::borrow(int size) {
    char *r;
    if (size + (this->_current - this->_pool) > (this->_size)) { // current pool not enough, borrow from next pool
        // if next pool is nullptr, create next pool
        if (this->_next == nullptr) {
            if (size > this->_size) { // too big, create (size) pool
                this->_next = new LinearMemoryPool(size);
            } else { // not too big, create (this->_size) pool
                this->_next = new (this->_pool + this->_size) LinearMemoryPool(this->_size);
            }
        }
        r = this->_next->borrow(size);
    } else {
        r = this->_current;
        this->_current += size;
    }
    return r;
}

void LinearMemoryPool::restore(char *addr) {
    // cannot restore
}

void LinearMemoryPool::clear() {
    if (this->_next != nullptr) {
        this->_next->~LinearMemoryPool(); // call destructor manually, avoid delete a part of this->_pool
    }
    this->_current = this->_pool;
}
