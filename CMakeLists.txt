cmake_minimum_required(VERSION 3.6)
project(RedisKnockoff)

set(CMAKE_CXX_STANDARD 20)

add_library(network_lib
        Socket.h
        Socket.cpp
        Connection.h
        Connection.cpp
        Epoll.h
        Epoll.cpp
)

target_include_directories(network_lib PUBLIC .)

include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/50b8600c63c5487e901e2845a0f64d384a65f75d.zip
)

FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(run_tests
        Tests/testEpoll.cpp
)

target_link_libraries(run_tests PRIVATE
        network_lib
        GTest::gtest
        GTest::gtest_main
)

add_executable(RedisKnockoff
        main.cpp
)

target_link_libraries(RedisKnockoff PRIVATE
        network_lib 
)

add_executable(RedisServer
        server.cpp
)

target_link_libraries(RedisServer
        network_lib
)

add_executable(RedisClient
        client.cpp
)

target_link_libraries(RedisClient
        network_lib
)