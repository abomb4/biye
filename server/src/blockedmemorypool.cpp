#include "blockedmemorypool.h"

BlockedMemoryPool::BlockedMemoryPool(const int blocksize, const int blockmax) {
    this->_bsize = blocksize;
    this->_bmax = blockmax;
    this->_allocs = 0;
    this->_borrows = 0;
};
BlockedMemoryPool::~BlockedMemoryPool() {

};
Memory BlockedMemoryPool::borrow() {
    _operating.lock();
    _operating.unlock();
}

void BlockedMemoryPool::restore(Memory &m) {
    _operating.lock();
    _operating.unlock();
}
