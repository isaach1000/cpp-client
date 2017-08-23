cmake_minimum_required(VERSION 3.0)

project(jaeger-client-cpp)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

if(NOT CMAKE_VERSION VERSION_LESS 3.1)
    set(CMAKE_CXX_STANDARD 11)
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -pedantic -pthread")

include(CTest)
enable_testing()
find_package(GTest REQUIRED)

find_package(Thrift REQUIRED)
include_directories(${THRIFT_INCLUDE_DIR})

include_directories(thrift_gen)

file(GLOB THRIFT_GEN_SRC thrift_gen/*.cpp)
list(FILTER THRIFT_GEN_SRC EXCLUDE REGEX ".*\\.skeleton\\.cpp")
add_library(jaeger ${THRIFT_GEN_SRC})

add_subdirectory(core)
