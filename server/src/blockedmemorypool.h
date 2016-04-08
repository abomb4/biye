#ifndef BLOCKEDMEMORYPOOL_H
#define BLOCKEDMEMORYPOOL_H

#include <mutex>
#include "memorypool.h"

class BlockedMemoryPool : public MemoryPool
{
public:
    BlockedMemoryPool(const int blocksize, const int blockmax);
    ~BlockedMemoryPool();
    Memory borrow();
    void restore(Memory &m);
private:
    char **_pool; // pool pointer
    int _bsize, _bmax; // block size and max block sum
    int _allocs; // already allocated blocks
    int _borrows; // borrowed blocks sum

    std::mutex _operating;
};

#endif // BLOCKEDMEMORYPOOL_H
