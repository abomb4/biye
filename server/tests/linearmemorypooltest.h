#ifndef LINEARMEMORYPOOLTEST_H
#define LINEARMEMORYPOOLTEST_H

#include <iostream>
#include <cstring>
#include <cmath>

#include <cppunit/CompilerOutputter.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/extensions/HelperMacros.h>

#include "linearmemorypool.h"

using std::cout;
using std::endl;

class LinearMemoryPoolTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE( LinearMemoryPoolTest );
    CPPUNIT_TEST( testBorrow );
    CPPUNIT_TEST( testOutedBorrow );
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() {
    };
    void tearDown() {
    };

    void testBorrow() {
        MemoryPool *_pool = new LinearMemoryPool(1024);
        char *a = _pool->borrow(16);
        for (int i = 0; i < 16; i++) {
            a[i] = i + 'A';
        }
        a = _pool->borrow(16);
        for (int i = 0; i < 16; i++) {
            a[i] = i + 'A';
        }
        _pool->clear();
        a = _pool->borrow(32);
        char *b = new char[33];
        memcpy(b, a, 32);
        b[32] = '\0';
        cout << b << endl;
        CPPUNIT_ASSERT(strcmp(b, "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP") == 0);
        delete (LinearMemoryPool*)_pool;
    };
    void testOutedBorrow() {
        MemoryPool *_pool = new LinearMemoryPool(1024);
        char *start = _pool->borrow(1023);
        char *end = _pool->borrow(1);
        CPPUNIT_ASSERT(end - start == 1023);
        _pool->clear();
        char *a = _pool->borrow(1024);
        char *b = _pool->borrow(32);
        CPPUNIT_ASSERT(abs(b - a) > 1024);
        delete (LinearMemoryPool*)_pool;
    }

private:
};
// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( LinearMemoryPoolTest );

#endif // LINEARMEMORYPOOLTEST_H
