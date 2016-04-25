cmake_minimum_required (VERSION 3.0)

# C++ 11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-long-long")

find_package (Threads)

# Find mysql library
find_library(MYSQL_LIBRARIES NAMES mysqlclient
    PATHS
    /usr/lib/mysql
    /usr/local/lib/mysql
    /usr/local/mysql/lib
    /opt/local/lib/mysql*/mysql
    )

# Find mysql++ library
find_library(MYSQLPP_LIBRARIES NAMES mysqlpp
    PATHS
    /usr/lib/
    /usr/local/lib/
    )

# include list
include_directories(./include)
add_library(spdlog INTERFACE)
target_include_directories(
    spdlog
    INTERFACE
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>"
    "$<INSTALL_INTERFACE:include>"
)

# Source list
add_library(libFxServer
    config.cpp
    memorypool.h
    blockedmemorypool.cpp
    linearmemorypool.cpp
    fxcjson.cpp
    fxcjson.h
    mysqlconnectionpool.cpp
    mysqlconnector.cpp
    fxmessage.cpp
    fxserver.cpp
    serverlistener.cpp
    clientconnection.cpp
    usermanager.cpp
    user.h
    usersession.cpp
)

add_executable(fxserver main.cpp)

target_link_libraries (fxserver ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries (fxserver ${MYSQL_LIBRARIES})
target_link_libraries (fxserver ${MYSQLPP_LIBRARIES})

target_link_libraries (fxserver libFxServer)

# utils
add_subdirectory(fxutils)
# target_link_libraries(fxserver fxutils)
