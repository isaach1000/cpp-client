cmake_minimum_required(VERSION 3.0)

include("cmake/HunterGate.cmake")
HunterGate(URL "https://github.com/ruslo/hunter/archive/v0.19.89.tar.gz"
           SHA1 "98bf8e401fff1394e8df0a757772265511dd9084")

project(jaeger-client-cpp)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

if(NOT CMAKE_VERSION VERSION_LESS 3.1)
    set(CMAKE_CXX_STANDARD 11)
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -pedantic -pthread")

# Soon to be Boost component.
hunter_add_package(Beast)
find_package(Beast CONFIG REQUIRED)
list(APPEND LIBS Beast::Beast)

hunter_add_package(Boost COMPONENTS system thread)
find_package(Boost CONFIG REQUIRED COMPONENTS system thread)
list(APPEND LIBS Boost::system Boost::thread)

hunter_add_package(nlohmann_json)
find_package(nlohmann_json CONFIG REQUIRED)
list(APPEND LIBS nlohmann_json)

include(CTest)
enable_testing()
hunter_add_package(GTest)
find_package(GTest CONFIG REQUIRED)

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
target_link_libraries(jaeger ${LIBS})

add_executable(UnitTest ${TEST_SRC})
target_link_libraries(UnitTest GTest::main jaeger)
add_test(NAME UnitTest
         COMMAND UnitTest)
if(CMAKE_BUILD_TYPE MATCHES Debug)
    setup_target_for_coverage(NAME UnitTestCoverage
                              EXECUTABLE UnitTest
                              DEPENDENCIES UnitTest)
endif()
