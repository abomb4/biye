#ifndef MEMORYPOOL_H
#define MEMORYPOOL_H

#include "memory.h"

class MemoryPool
{
public:
    virtual Memory borrow() = 0;
    virtual void restore(Memory &m) = 0;
};

#endif // MEMORYPOOL_H
