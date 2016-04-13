#ifndef BLOCKEDMEMORYPOOL_H
#define BLOCKEDMEMORYPOOL_H

#include "memorypool.h"
#include <mutex>
#include <spdlog/spdlog.h>

///
/// \brief 分块的内存池，一次只能申请一个块。
/// 申请超过块数限制的内存时会新建一个同样属性的BlockedMemoryPool。
/// 使用了C++ 11线程锁。
/// 内存结构：
///     当bsize=8 bmax=2时，borrow2次，restore1次时，
///     内存pool为[DDDDDDDD1xxxxxxxx000000000]
///
class BlockedMemoryPool : public MemoryPool
{
public:
    BlockedMemoryPool(const int blocksize, const int blockmax);
    ~BlockedMemoryPool();
    char* borrow(int size); // size is useless
    void restore(char *addr); // MUST RESTORE CORRECT ADDR!!!!!!!!
    void clear();
private:
    static std::shared_ptr<spdlog::logger> _logger;

    char *_pool; // pool pointer
    char **_borrowable; // borrowable block stack
    char *_tail; //
    int _bsize, _bmax; // block size and max block sum
    int _avaliable; // borrowed blocks sum
    BlockedMemoryPool *_next; // next pool pointer

    std::mutex _operating;
};

#endif // BLOCKEDMEMORYPOOL_H
