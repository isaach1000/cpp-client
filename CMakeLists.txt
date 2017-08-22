cmake_minimum_required(VERSION 2.8)

project(jaeger-client-cpp)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
set(CMAKE_CXX_FLAGS "-std=c++11 -Wall -Werror")

find_package(Thrift REQUIRED)
include_directories(${THRIFT_INCLUDE_DIR})

include_directories(thrift_gen)

file(GLOB THRIFT_GEN_SRC thrift_gen/*.cpp)
list(FILTER THRIFT_GEN_SRC EXCLUDE REGEX ".*\\.skeleton\\.cpp")
add_library(jaeger ${THRIFT_GEN_SRC})

add_subdirectory(core)

target_link_libraries(jaeger ${THRIFT_LIBS})
