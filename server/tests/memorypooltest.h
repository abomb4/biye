#ifndef MEMORYPOOLTEST_H
#define MEMORYPOOLTEST_H

#include <iostream>
#include <cstring>
#include <thread>
#include <mutex>

#include <cppunit/CompilerOutputter.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/extensions/HelperMacros.h>

#include "blockedmemorypool.h"

using std::cout;
using std::endl;
using std::thread;
using std::mutex;

class MemoryPoolTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE( MemoryPoolTest );
    CPPUNIT_TEST( testThreadSafe );
    CPPUNIT_TEST( testMaximum );
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() {
        p1024 = new BlockedMemoryPool(1024, 2);
        p10240 = new BlockedMemoryPool(10240, 2);
        p65536 = new BlockedMemoryPool(65536, 2);

        _mems = 0;
    };
    void tearDown() {
        delete p1024;
        delete p10240;
        delete p65536;
    };

    void testThreadSafe() {

    };

    void testMaximum() {
        Memory m1 = p1024->borrow();
        _mems++;
        Memory m2 = p1024->borrow();
        _mems++;
        Memory m3 = p1024->borrow();
        _mems++;
        cout << "M3 borrowed" << endl;
    }

private:
    MemoryPool *p1024, *p10240, *p65536;

    mutex _stopthreads;
    int _mems;
    void _doMsx() {
        Memory m = p1024->borrow();
        _mems++;
        _stopthreads.lock();
    }
};
// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( MemoryPoolTest );

#endif // MEMORYPOOLTEST_H
