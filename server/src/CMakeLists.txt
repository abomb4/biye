cmake_minimum_required (VERSION 3.5)

# C++ 11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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

# Source list
add_library(FXMemoryPool memorypool.h blockedmemorypool.cpp memory.h)
add_library(FxMysqlConnector mysqlconnectionpool.cpp mysqlconnector.cpp)
set(srclist
    config.cpp
    main.cpp
    serverlistener.cpp
    clientconnection.cpp
    usermanager.cpp
    user.cpp
    usersession.cpp
    fxmessage.cpp
    )

add_executable(fxserver ${srclist})

target_link_libraries (fxserver ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries (fxserver ${MYSQL_LIBRARIES})
target_link_libraries (fxserver ${MYSQLPP_LIBRARIES})

target_link_libraries (fxserver FXMemoryPool)
target_link_libraries (fxserver FxMysqlConnector)

# utils
add_subdirectory(fxutils)
# target_link_libraries(fxserver fxutils)
