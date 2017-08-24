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
include_directories(${GTEST_INCLUDE_DIRS})

find_package(Thrift REQUIRED)
include_directories(${THRIFT_INCLUDE_DIR})

include_directories(thrift_gen)

file(GLOB THRIFT_GEN_SRC thrift_gen/*.cpp)
list(FILTER THRIFT_GEN_SRC EXCLUDE REGEX ".*\\.skeleton\\.cpp")
add_library(jaeger ${THRIFT_GEN_SRC})

if(CMAKE_BUILD_TYPE MATCHES Debug)
    include(CodeCoverage)
    APPEND_COVERAGE_COMPILER_FLAGS()
    set(COVERAGE_EXCLUDES "thrift_gen/*"
                          "${THRIFT_INCLUDE_DIR}/*"
                          "/usr/include/c++/6/*"
                          "/usr/include/c++/6/bits/*"
                          "/usr/include/c++/6/ext/*")
    foreach(inc_dir IN LISTS GTEST_INCLUDE_DIRS)
        list(APPEND COVERAGE_EXCLUDES "${inc_dir}/gtest/*"
                                      "${inc_dir}/gtest/internal/*")
    endforeach()
    message(STATUS "COVERAGE_EXCLUDES=${COVERAGE_EXCLUDES}")
endif()

 add_subdirectory(core)
