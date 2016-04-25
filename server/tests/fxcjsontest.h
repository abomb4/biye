#ifndef FXCJSONTEST_H
#define FXCJSONTEST_H

#include <iostream>
#include <cstring>

#include <cppunit/CompilerOutputter.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/extensions/HelperMacros.h>

#include "linearmemorypool.h"
#include "fxcjson.h"
#include <thread>

using std::cout;
using std::endl;

class TTTTTTTTTTTTTT {
public:
    TTTTTTTTTTTTTT() {
        this->_pool = new LinearMemoryPool(1024);
        this->_ptr = nullptr;
        this->_t = std::thread([=] { this->_start(); });
    }

    void *getPtr() { return (void *)this->_ptr; }
    void thread_start() {
    }
    void join() {
        _t.join();
    }

private:
    cJSON *_ptr;
    std::thread _t;
    LinearMemoryPool *_pool;

    void _start() {
        cJSON_set_pool(this->_pool);
        _ptr = cJSON_CreateNull();
    }
};

class FxCJsonTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE( FxCJsonTest );
    CPPUNIT_TEST( testThreadSafe );
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() {
    };
    void tearDown() {
    };

    void testThreadSafe();

private:
};
// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( FxCJsonTest );

void FxCJsonTest::testThreadSafe() {
    TTTTTTTTTTTTTT t1, t2, t3;
    t1.thread_start();
    t2.thread_start();
    t3.thread_start();
    t1.join();
    t2.join();
    t3.join();
    CPPUNIT_ASSERT(t1.getPtr() != t2.getPtr());
    CPPUNIT_ASSERT(t1.getPtr() != t3.getPtr());
    CPPUNIT_ASSERT(t3.getPtr() != t2.getPtr());
}
#endif // FXCJSONTEST_H
