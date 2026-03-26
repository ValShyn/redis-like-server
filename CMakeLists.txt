cmake_minimum_required(VERSION 3.10)
project(RedisClone)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

file(GLOB SOURCES "*.cpp")

add_executable(redis_server ${SOURCES})