cmake_minimum_required(VERSION 3.16.5)
project(nio_cpp)

set(CMAKE_CXX_STANDARD 14)

add_executable(nio_cpp main.cpp example/server-1.cpp include/common.h)
