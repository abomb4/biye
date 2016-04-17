#ifndef LINEARMEMORYPOOL_H
#define LINEARMEMORYPOOL_H

#define LINEARMEMORYPOOL_MIN_SIZE 32

#include "memorypool.h"

///
/// non thread safe
///
class LinearMemoryPool : public MemoryPool
{
public:
    LinearMemoryPool(int size);
    ~LinearMemoryPool();
    char* borrow(int size);
    void restore(char *addr);
    void clear();
private:
    char *_pool;
    char *_current;
    int _size;
    LinearMemoryPool *_next;
};

#endif // LINEARMEMORYPOOL_H
