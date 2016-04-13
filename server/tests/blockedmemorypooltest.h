#ifndef BLOCKEDMEMORYPOOLTEST_H
#define BLOCKEDMEMORYPOOLTEST_H

#include <iostream>
#include <cstring>
#include <cmath>

#include <cppunit/CompilerOutputter.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/extensions/HelperMacros.h>

#include "blockedmemorypool.h"

using std::cout;
using std::endl;

class BlockedMemoryPoolTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE( BlockedMemoryPoolTest );
    CPPUNIT_TEST( testBorrow );
    CPPUNIT_TEST( testOutedBorrow );
    CPPUNIT_TEST( testRestore );
    CPPUNIT_TEST( testOutedRestore );
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() {
    };
    void tearDown() {
    };

    void testBorrow() {
        BlockedMemoryPool *p = new BlockedMemoryPool(32, 2);
        char *addr1 = p->borrow(32);
        char *addr2 = p->borrow(32);
        cout << "abs: " << abs(addr1 - addr2) << endl;
        CPPUNIT_ASSERT(abs(addr1 - addr2) == 32);
        delete p;
    };

    void testOutedBorrow() {
        BlockedMemoryPool *p = new BlockedMemoryPool(32, 2);
        char *addr1 = p->borrow(32);
        char *addr2 = p->borrow(32);
        char *addr3 = p->borrow(32);
        cout << "abs13: " << abs(addr1 - addr3) << endl;
        cout << "abs23: " << abs(addr2 - addr3) << endl;
        CPPUNIT_ASSERT(abs(addr2 - addr3) != 32);
        CPPUNIT_ASSERT(abs(addr1 - addr3) != 32);
        delete p;
    };

    void testRestore() {
        BlockedMemoryPool *p = new BlockedMemoryPool(32, 2);
        char *a1 = p->borrow(32);
        char *b1 = p->borrow(32);
        p->restore(a1);
        p->restore(b1);
        char *b2 = p->borrow(32);
        char *a2 = p->borrow(32);
        CPPUNIT_ASSERT(a1 == a2);
        CPPUNIT_ASSERT(b1 == b2);

        delete p;
    }

    void testOutedRestore() {
        BlockedMemoryPool *p = new BlockedMemoryPool(32, 1);
        char *a1 = p->borrow(32);
        char *b1 = p->borrow(32);
        p->restore(a1);
        p->restore(b1);
        char *a2 = p->borrow(32);
        char *b2 = p->borrow(32);
        CPPUNIT_ASSERT(a1 == a2);
        CPPUNIT_ASSERT(b1 == b2);

        delete p;
    }

private:
};
// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( BlockedMemoryPoolTest );

#endif // BLOCKEDMEMORYPOOLTEST_H
