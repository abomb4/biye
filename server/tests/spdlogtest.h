#ifndef SPDLOGTEST_H
#define SPDLOGTEST_H

#include <iostream>
#include <cstring>
#include <cmath>

#include <cppunit/CompilerOutputter.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/extensions/HelperMacros.h>

#include <spdlog/spdlog.h>

using std::cout;
using std::endl;

class SpdLogTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE( SpdLogTest );
    CPPUNIT_TEST( testConsole );
    CPPUNIT_TEST( testMultiSinks );
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() {
    };
    void tearDown() {
    };

    void testMultiSinks() {    }

    void testConsole() {
    }
private:
};
// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( SpdLogTest );

#endif // SPDLOGTEST_H
