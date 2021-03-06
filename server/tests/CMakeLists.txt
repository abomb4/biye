cmake_minimum_required(VERSION 3.0)
enable_testing()

# C++ 11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package (Threads)

# Find cppunit library
find_library(CPPUNIT_LIBRARIES NAMES cppunit
    PATHS
    /usr/lib
    /usr/local/lib
    )


include_directories(../src)
include_directories(../src/include)

add_library( FXTESTS STATIC
    fxmessagetest.h
    linearmemorypooltest.h
    blockedmemorypooltest.h
    mysqlconnectortest.h
    spdlogtest.h
    fxcjsontest.h
    usermanagertest.h
)
set_target_properties(FXTESTS PROPERTIES LINKER_LANGUAGE CXX)

add_executable(test_main main.cpp)
target_link_libraries (test_main ${CMAKE_THREAD_LIBS_INIT})

target_link_libraries(test_main ${CPPUNIT_LIBRARIES})
target_link_libraries(test_main ${MYSQLPP_LIBRARIES})

target_link_libraries (test_main libFxServer)
add_test(memory_test test_main)
