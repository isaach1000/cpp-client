cmake_minimum_required(VERSION 3.0)

project(jaeger-client-cpp)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

if(NOT CMAKE_VERSION VERSION_LESS 3.1)
    set(CMAKE_CXX_STANDARD 11)
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -pedantic -pthread")

find_package(Boost 1.54 REQUIRED COMPONENTS system thread)
include_directories(${Boost_INCLUDE_DIRS})

include(CTest)
enable_testing()
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

find_package(Thrift REQUIRED)
include_directories(${THRIFT_INCLUDE_DIR})

if(CMAKE_BUILD_TYPE MATCHES Debug)
    include(CodeCoverage)
    APPEND_COVERAGE_COMPILER_FLAGS()
endif()

include_directories("include")

file(GLOB SRC "src/uber/jaeger/*.cpp")
file(GLOB TEST_SRC "src/uber/jaeger/*Test.cpp")
foreach(SUBDIR "metrics" "propagation" "reporters" "samplers" "utils")
    file(GLOB SUBDIR_SRC "src/uber/jaeger/${SUBDIR}/*.cpp")
    list(APPEND SRC ${SUBDIR_SRC})
    file(GLOB SUBDIR_TEST_SRC "src/uber/jaeger/${SUBDIR}/*Test.cpp")
    list(APPEND TEST_SRC ${SUBDIR_TEST_SRC})
endforeach(SUBDIR)
list(FILTER SRC EXCLUDE REGEX ".*Test\\.cpp")

file(GLOB THRIFT_GEN_SRC src/uber/jaeger/thrift_gen/*.cpp)
list(FILTER THRIFT_GEN_SRC EXCLUDE REGEX ".*\\.skeleton\\.cpp")

add_library(jaeger ${SRC})
target_link_libraries(jaeger ${Boost_SYSTEM_LIBRARY} ${Boost_THREAD_LIBRARY})

add_executable(UnitTest ${TEST_SRC})
target_link_libraries(UnitTest ${GTEST_BOTH_LIBRARIES} jaeger)
add_test(NAME UnitTest
         COMMAND UnitTest)
if(CMAKE_BUILD_TYPE MATCHES Debug)
    setup_target_for_coverage(NAME UnitTestCoverage
                              EXECUTABLE UnitTest
                              DEPENDENCIES UnitTest)
endif()
