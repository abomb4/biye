#ifndef FXMESSAGETEST_H
#define FXMESSAGETEST_H

#include <iostream>
#include <cstring>

#include <cppunit/CompilerOutputter.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/extensions/HelperMacros.h>

#include "linearmemorypool.h"
#include "fxmessage.h"
using namespace FxChat;

using std::cout;
using std::endl;

class FxMessageTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE( FxMessageTest );
    CPPUNIT_TEST( testDivide );
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() {
    };
    void tearDown() {
    };

    void testDivide() ;

private:
};
// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( FxMessageTest );

void FxMessageTest::testDivide() {
    LinearMemoryPool *pool = new LinearMemoryPool(102410);
    FxMessage *m = new FxMessage();
    FxMessageParam *p = new FxMessageParam();
    int lname = 10240;
    char *name = pool->borrow(lname);
    memset(name, '0', lname);
    p->setName(name, lname);
    int lval = 40960;
    char *val = pool->borrow(lval);
    memset(val, '1', lval);
    p->setVal(val, lval);
    m->addParam(p);
    char **packages;
    int *plengths;
    int packsum = m->toPackages(packages, plengths, pool);
    cout << "packsum: " << packsum << endl;
    pool->clear();
}

#endif // FXMESSAGETEST_H
