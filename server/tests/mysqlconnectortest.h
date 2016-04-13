#ifndef MYSQLCONNECTORTEST_H
#define MYSQLCONNECTORTEST_H

#include <iostream>
#include <cstring>
#include <cmath>

#include <cppunit/CompilerOutputter.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/extensions/HelperMacros.h>

#include "config.h"
#include "mysqlconnector.h"

using std::cout;
using std::endl;

class MysqlConnectorTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE( MysqlConnectorTest );
    CPPUNIT_TEST( testStmt );
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() {
    };
    void tearDown() {
    };

    void testStmt() {
        Config::load("/home/x4x/Workspace/biye/server/src/default.cfg");
        MysqlConnector *c = MysqlConnector::getInstance();
        mysqlpp::Connection *con = c->getConnection();
        mysqlpp::Query q = con->query();
        q << "select id, name, password, email from user where name = %0q and password = %1q";
        q.parse();
        mysqlpp::StoreQueryResult res = q.store("x4x", "cao123");
        CPPUNIT_ASSERT(res.size() == 1);
        mysqlpp::Row row = res[0];
        cout << "id: " << row[0]
             << ", name: " << row[1]
             << ", password: " << row[2]
             << ", email: " << row[3]
             << "." << endl;
    };
private:
};
// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( MysqlConnectorTest );

#endif // MYSQLCONNECTORTEST_H
