cmake_minimum_required(VERSION 3.16.5)
project(nio_cpp)

set(CMAKE_CXX_STANDARD 14)

add_executable(main main.cpp)
add_executable(server-1 example/server-1.cpp)
add_executable(epoll-server example/epoll-server.cpp)
add_executable(select-server example/select-server.cpp)