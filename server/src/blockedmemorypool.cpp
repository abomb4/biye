#include <iostream>
using std::cout;
using std::endl;

#include "blockedmemorypool.h"

std::shared_ptr<spdlog::logger> BlockedMemoryPool::_logger;

BlockedMemoryPool::BlockedMemoryPool(const int blocksize, const int blockmax) {
    if (BlockedMemoryPool::_logger == nullptr)
        BlockedMemoryPool::_logger = spdlog::get("BlockedMemoryPool");
    _logger->debug("({}) A BlockedMemoryPool created, block size: {}, block max: {}.",
                   (void*)this, blocksize, blockmax);
    this->_next = nullptr; // 1

    this->_bsize = blocksize; // 2
    this->_bmax = blockmax; // 3
    this->_avaliable = this->_bmax; // 4

    int ablesize = this->_bsize * this->_bmax;
    // fullsize is borrow able size + next pool object size + stack size
    int fullsize = ablesize + sizeof(BlockedMemoryPool) + sizeof(char*) * this->_bmax;
    this->_pool = new char[fullsize]; // 5
    bzero(this->_pool, fullsize);

    this->_tail = this->_pool + ablesize; // 6

    this->_borrowable = new (this->_tail + sizeof(BlockedMemoryPool)) char*[this->_bmax]; // 7
    // build default stack
    for (int i = 0; i < this->_bmax; i++) {
        _borrowable[i] = _pool + i * this->_bsize;
    }
};

BlockedMemoryPool::~BlockedMemoryPool() {
    this->clear();
    delete[] this->_pool;
    // delete[] this->_borrowable;
    _logger->debug("({}) A BlockMemoryPool destroied.", (void*)this);
};

void BlockedMemoryPool::clear() {
    this->_operating.lock();
    _logger->debug("({}) Clear()", (void*)this);
    if (this->_next != nullptr)
        this->_next->~BlockedMemoryPool(); // call destructor manually, avoid delete a part of this->_pool
    this->_avaliable = this->_bmax;

    // build default stack
    for (int i = 0; i < this->_bmax; i++) {
        _borrowable[i] = _pool + i * this->_bsize;
    }
    this->_operating.unlock();
}

char* BlockedMemoryPool::borrow(int size) {
    this->_operating.lock();
    if (size != this->_bsize)
        _logger->warn("({}) borrow() Requies {} Byte but block size is {}! THIS SHOULD NOT HAPPEND!!",
                      (void*)this, size, this->_bsize);

    char *re;
    if (this->_avaliable > 0) { // have avaliable block
        re = this->_borrowable[_avaliable - 1];
        this->_avaliable--;
        _logger->debug("({}) borrow() Borrowed memory is {}, current {} block avaliable.",
                       (void*)this, (void*)re, this->_avaliable);
    } else { // no block avaliable
        _logger->debug("({}) borrow() No block avaliable, create new BlockedMemoryPool and borrow from it.",
                       (void*)this);
        if (this->_next == nullptr)
            this->_next = new (this->_tail) BlockedMemoryPool(this->_bsize, this->_bmax);
        re = this->_next->borrow(size);
    }
    this->_operating.unlock();
    return re;
}

void BlockedMemoryPool::restore(char *addr) {
    this->_operating.lock();
    if (addr >= this->_pool && addr < this->_tail) { // in this pool
        this->_borrowable[this->_avaliable] = addr;
        this->_avaliable++;
        _logger->debug("({}) restore() Addr {} restored, current avaliable blocks: {}.",
                       (void*)this, (void*)addr, this->_avaliable);
    } else if (this->_next != nullptr) { // may in next pool
        _logger->debug("({}) restore() Addr {} is not in this pool, try restore to next pool({}).",
                       (void*)this, (void*)addr, (void*)this->_next);
        this->_next->restore(addr);
    } else { // not allocate by memory pool
        _logger->warn("({}) restore() Addr {} is NOT BORROWED FROM THIS POOL!!! WILL DELETE IT!",
                      (void*)this, (void*)addr);
        delete addr; // XXX
    }
    this->_operating.unlock();
}
