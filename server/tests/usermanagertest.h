#ifndef USERMANAGERTEST_H
#define USERMANAGERTEST_H

#include <iostream>
#include <cstring>

#include <cppunit/CompilerOutputter.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/extensions/HelperMacros.h>

#include "linearmemorypool.h"
#include "mysqlconnector.h"
#include "config.h"
#include "usermanager.h"
#include "fxcjson.h"

using std::cout;
using std::endl;

class UserManagerTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE( UserManagerTest );
    CPPUNIT_TEST( testFullList );
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() {
    };
    void tearDown() {
    };

    void testFullList() ;

private:
};
// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( UserManagerTest );

void UserManagerTest::testFullList() {
    Config::load("/home/x4x/Workspace/biye/server/src/default.cfg");
    LinearMemoryPool *pool = new LinearMemoryPool(10240);
    User *list;
    int count = UserManager::getInstance().getFullList(list, pool);
    cJSON_set_pool(pool);
    cJSON *array = cJSON_CreateArray();
    for (int i = 0; i < count; i++) {
        cJSON *obj = cJSON_CreateObject();
        cJSON_AddNumberToObject(obj, "id", list[i].id());
        cJSON_AddStringToObject(obj, "name", list[i].name().c_str());
        cJSON_AddNumberToObject(obj, "department", list[i].department());
        cJSON_AddStringToObject(obj, "true_name", list[i].trueName().c_str());
        cout << "id: " << list[i].id()
             << ", name: " << list[i].name()
             << ", department:" << list[i].department()
             << ", true_name:" << list[i].trueName()
             << endl;
        cJSON_AddItemToArray(array, obj);
    }
    cout << "JSON:: " << cJSON_PrintUnformatted(array) << endl;

    pool->clear();
}

#endif // USERMANAGERTEST_H
