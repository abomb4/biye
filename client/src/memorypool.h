#ifndef MEMORYPOOL_H
#define MEMORYPOOL_H

#include "memory.h"

class MemoryPool
{
public:
    virtual char* borrow(int size) = 0; // UNABLE TO FREE/DELETE BORROWED MEMORY!
    virtual void restore(char *addr) = 0;
    virtual void clear() = 0; // CLEAR IS NOT DELETE! ONLY initialize!
};

#endif // MEMORYPOOL_H
