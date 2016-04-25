#ifndef LINEARMEMORYPOOL_H
#define LINEARMEMORYPOOL_H

#define LINEARMEMORYPOOL_MIN_SIZE 32

#include "memorypool.h"

#include <spdlog/spdlog.h>

///
/// non thread safe
///
class LinearMemoryPool : public MemoryPool
{
public:
    LinearMemoryPool(int size);
    LinearMemoryPool(int size, int basesize);
    ~LinearMemoryPool();
    char* borrow(int size);
    void restore(char *addr);
    void clear();
private:
    static std::shared_ptr<spdlog::logger> _logger;

    char *_pool;
    char *_current;
    int _size;
    int _basesize;
    LinearMemoryPool *_next;
};

#endif // LINEARMEMORYPOOL_H
