#ifndef MEMORY_H
#define MEMORY_H

#include <cstring>

class Memory
{
public:
    Memory(char *ptr, int size, int id) {
        this->_poolptr = ptr;
        // this->_currentptr = this->_poolptr;
        this->_size = size;
        this->_currentlen = 0;
        this->_id = id;
        this->_usable = true;
    }

    void __restore() {
        this->_usable = false;
    }

    const int __id() const {
        return this->_id;
    }

    const char *ptr() const {
        return this->_poolptr;
    }

    const int size() const {
        return this->_size;
    }

    const int curlen() const {
        return this->_currentlen;
    }

    bool append(const char *newchar, const int length) {
        if (!this->_usable)
            return false;
        if (this->_currentlen + length > this->_size)
            return false;
        char *p = this->_poolptr + this->_currentlen;
        memcpy(p, newchar, length);
        this->_currentlen += length;
        return true;
    }

private:
    char *_poolptr; // pool's memory pointer
    int _size, _currentlen; // maximum size and current used length, curlen used for append()
    int _id;
    bool _usable;
};

#endif // MEMORY_H
