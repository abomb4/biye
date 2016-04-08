#ifndef MEMORYTEST_H
#define MEMORYTEST_H

#include <iostream>
#include <cstring>

#include <cppunit/CompilerOutputter.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/extensions/HelperMacros.h>

#include "memory.h"

using std::cout;
using std::endl;

class MemoryTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE( MemoryTest );
    CPPUNIT_TEST( testAppend );
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() {
    };
    void tearDown() {
    };

    void testAppend() {
        char *allmem = new char[100];
        Memory m(allmem, 10, 0);
        m.append("xxxx", 4);
        CPPUNIT_ASSERT(m.curlen() == 4);
        m.append("yyyyyy", 6);
        CPPUNIT_ASSERT(m.curlen() == 10);
        char str[40];
        memcpy(str, m.ptr(), 10);
        str[10] = '\0';
        CPPUNIT_ASSERT(strcmp(str, "xxxxyyyyyy") == 0);
        CPPUNIT_ASSERT(m.append("x", 1) == false);
        CPPUNIT_ASSERT(m.curlen() == 10);
    };

private:
};
// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( MemoryTest );

#endif // MEMORYTEST_H
