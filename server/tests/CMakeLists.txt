cmake_minimum_required(VERSION 3.5)
enable_testing()

# C++ 11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find cppunit library
find_library(CPPUNIT_LIBRARIES NAMES cppunit
    PATHS
    /usr/lib
    /usr/local/lib
    )


include_directories(../src)

add_library( FXTESTS STATIC
    memorytest.h
    memorypooltest.h
)
set_target_properties(FXTESTS PROPERTIES LINKER_LANGUAGE CXX)

add_executable(test_main main.cpp)

target_link_libraries(test_main ${CPPUNIT_LIBRARIES})
target_link_libraries(test_main FXMemoryPool)

add_test(memory_test test_main)
