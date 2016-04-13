#include <iostream>
using std::cout;
using std::endl;

#include "linearmemorypool.h"

std::shared_ptr<spdlog::logger> LinearMemoryPool::_logger;

LinearMemoryPool::LinearMemoryPool(int size) {
    if (LinearMemoryPool::_logger == nullptr)
        LinearMemoryPool::_logger = spdlog::get("LinearMemoryPool");
    _logger->debug("({}) A LinearMemoryPool created, pool size: {}.",
                   (void*)this, size);

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
    _logger->debug("({}) A LinearMemoryPool destroied.", (void*)this);
}

char* LinearMemoryPool::borrow(int size) {
    char *r;
    if (size + (this->_current - this->_pool) > (this->_size)) { // current pool not enough, borrow from next pool
        // if next pool is nullptr, create next pool
        if (this->_next == nullptr) {
            _logger->debug("({}) borrow() Current pool is not enough(current: {}/{}), need {}, create new pool.",
                          (void*)this, this->_size, (long)(this->_current - this->_pool), size);
            if (size > this->_size) { // too big, create (size) pool
                _logger->debug("({}) A Big pool required, create an {} byte pool.");
                this->_next = new LinearMemoryPool(size);
            } else { // not too big, create (this->_size) pool
                _logger->debug("({}) Create new pool.", (void*)this);
                this->_next = new (this->_pool + this->_size) LinearMemoryPool(this->_size);
            }
        }
        r = this->_next->borrow(size);
        _logger->debug("({}) borrow() Memory {}, size: {}, borrowed from next pool {}.",
                       (void*)this, (void*)r, size, (void*)this->_next);
    } else {
        r = this->_current;
        this->_current += size;
        _logger->debug("({}) borrow() Borrowed memory is {}, size: {}, current {} byte avaliable.",
                       (void*)this, (void*)r, size, this->_size - (long)(this->_current - this->_pool));
    }
    return r;
}

void LinearMemoryPool::restore(char *addr) {
    // cannot restore
    _logger->warn("({}) restore() LinearMemoryPool not supports restore()!", (void*)this);
}

void LinearMemoryPool::clear() {
    _logger->debug("({}) Clear();", (void*)this);
    if (this->_next != nullptr) {
        _logger->debug("({}) Destruct next pool({}).", (void*)this, (void*)this->_next);
        this->_next->~LinearMemoryPool(); // call destructor manually, avoid delete a part of this->_pool
    }
    this->_current = this->_pool;
}
