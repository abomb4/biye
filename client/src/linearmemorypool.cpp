#include <iostream>
#include <QDebug>
using std::cout;
using std::endl;

#include "linearmemorypool.h"

LinearMemoryPool::LinearMemoryPool(int size) {
    new(this) LinearMemoryPool(size, size);
}

LinearMemoryPool::LinearMemoryPool(int size, int basesize) {
    qDebug() << "LinearMemoryPool created, size " << size;

    if (size < LINEARMEMORYPOOL_MIN_SIZE)
        size = LINEARMEMORYPOOL_MIN_SIZE;
    this->_size = size;
    this->_basesize = basesize;
    this->_pool = new char[size + sizeof(LinearMemoryPool)];
    this->_current = this->_pool;
    this->_next = nullptr;
}

LinearMemoryPool::~LinearMemoryPool() {
    this->clear();
    delete[] this->_pool;
    qDebug() << "A LinearMemoryPool destroied.";
}

char* LinearMemoryPool::borrow(int size) {
    char *r;
    if (size + (this->_current - this->_pool) > (this->_size)) { // current pool not enough, borrow from next pool
        // if next pool is nullptr, create next pool
        if (this->_next == nullptr) {
            if (size > this->_basesize) { // too big, create (size + this->_basesize) pool
                this->_next = new LinearMemoryPool(size + this->_basesize, this->_basesize);
            } else { // not too big, create (this->_size) pool
                this->_next = new (this->_pool + this->_size) LinearMemoryPool(this->_basesize);
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
    qDebug() << "WARNING restore() LinearMemoryPool not supports restore()!";
}

void LinearMemoryPool::clear() {
    qDebug() << "LinearMemoryPool::clear()";
    if (this->_next != nullptr) {
        this->_next->~LinearMemoryPool(); // call destructor manually, avoid delete a part of this->_pool
        this->_next = nullptr;
    }
    this->_current = this->_pool;
}
