#include <iostream>
#include <cstring>

#include <cppunit/CompilerOutputter.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/extensions/HelperMacros.h>

#include "linearmemorypooltest.h"
#include "blockedmemorypooltest.h"
#include "mysqlconnectortest.h"

#include <spdlog/spdlog.h>

using std::cout;
using std::endl;
using std::cerr;

int main(int argc, char* argv[])
{
    try {
        std::vector<spdlog::sink_ptr> sinks;
        sinks.push_back(std::make_shared<spdlog::sinks::stdout_sink_st>());
        spdlog::set_level(spdlog::level::debug);
        // if new logger required, must add to here!
        const char *loglist[] = {
            "main",
            "ClientConnection",
            "ServerListener",
            "LinearMemoryPool",
            "BlockedMemoryPool",
            "UserSession",
            ""
        };
        for (int i = 0; strcmp("", loglist[i]) != 0; i++) {
            auto combined_logger = std::make_shared<spdlog::logger>(loglist[i], begin(sinks), end(sinks));
            combined_logger->set_level(spdlog::level::debug);
            spdlog::register_logger(combined_logger);
        }
    } catch (const spdlog::spdlog_ex& ex) {
        cerr << "Log failed: " << ex.what() << endl;
    }

    // Get the top level suite from the registry
    CppUnit::Test *suite = CppUnit::TestFactoryRegistry::getRegistry().makeTest();

    // Adds the test to the list of test to run
    CppUnit::TextUi::TestRunner runner;
    runner.addTest( suite );

    // Change the default outputter to a compiler error format outputter
    runner.setOutputter( new CppUnit::CompilerOutputter( &runner.result(),
                                                       std::cerr ) );
    // Run the tests.
    bool wasSucessful = runner.run();

    // Return error code 1 if the one of test failed.
    return wasSucessful ? 0 : 1;
}
