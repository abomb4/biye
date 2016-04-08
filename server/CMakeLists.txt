cmake_minimum_required(VERSION 3.5)
# project name
project(FXChat_Server)
# include path
# INCLUDE_DIRECTORIES(
#     include
# )

# C++ 11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

ADD_SUBDIRECTORY(src)
ADD_SUBDIRECTORY(tests)
