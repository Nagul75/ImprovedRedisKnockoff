cmake_minimum_required(VERSION 3.6)
project(RedisKnockoff)

set(CMAKE_CXX_STANDARD 20)

add_executable(RedisKnockoff
        main.cpp
        Socket.cpp
        Socket.h
        server.cpp
        client.cpp
)
