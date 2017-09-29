cmake_minimum_required(VERSION 3.0)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

include(HunterGate)
HunterGate(URL "https://github.com/ruslo/hunter/archive/v0.19.89.tar.gz"
           SHA1 "98bf8e401fff1394e8df0a757772265511dd9084")

project(jaeger-client-cpp)

if(NOT CMAKE_VERSION VERSION_LESS 3.1)
    set(CMAKE_CXX_STANDARD_REQUIRED true)
    set(CMAKE_CXX_STANDARD 11)
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -pedantic")
set(CMAKE_EXPORT_COMPILE_COMMANDS true)

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

hunter_add_package(spdlog)
find_package(spdlog CONFIG REQUIRED)
list(APPEND LIBS spdlog::spdlog)

include(CTest)
if(BUILD_TESTING)
    hunter_add_package(GTest)
    find_package(GTest CONFIG REQUIRED)
    add_definitions(-DGTEST_HAS_TR1_TUPLE=0 -DGTEST_USE_OWN_TR1_TUPLE=0)

    if(CMAKE_BUILD_TYPE MATCHES Debug)
        include(CodeCoverage)
        APPEND_COVERAGE_COMPILER_FLAGS()
        set(COVERAGE_EXCLUDES "${CMAKE_SOURCE_DIR}/src/uber/jaeger/thrift-gen/*"
                              "*Test.cpp")
    endif()
endif()

find_package(Thrift REQUIRED)
include_directories(${THRIFT_INCLUDE_DIR})

include_directories("src")

file(GLOB SRC "src/uber/jaeger/*.cpp")
file(GLOB TEST_SRC "src/uber/jaeger/*Test.cpp"
                   "src/uber/jaeger/testutils/*Test.cpp")
foreach(SUBDIR "metrics"
               "propagation"
               "reporters"
               "samplers"
               "utils")
    file(GLOB SUBDIR_SRC "src/uber/jaeger/${SUBDIR}/*.cpp")
    list(FILTER SUBDIR_SRC EXCLUDE REGEX ".*Test\\.cpp")
    list(APPEND SRC ${SUBDIR_SRC})
    file(GLOB SUBDIR_TEST_SRC "src/uber/jaeger/${SUBDIR}/*Test.cpp")
    list(APPEND TEST_SRC ${SUBDIR_TEST_SRC})
endforeach(SUBDIR)

add_subdirectory("src/uber/jaeger/thrift-gen")
list(APPEND LIBS thriftgen)

add_library(jaeger ${SRC})
target_link_libraries(jaeger ${LIBS})

if(BUILD_TESTING)
    add_subdirectory("${CMAKE_SOURCE_DIR}/src/uber/jaeger/testutils")
    add_executable(UnitTest ${TEST_SRC})
    target_link_libraries(UnitTest testutils GTest::main jaeger ${LIBS})
    add_test(NAME UnitTest
             COMMAND UnitTest)
    if(CMAKE_BUILD_TYPE MATCHES Debug)
        setup_target_for_coverage(NAME UnitTestCoverage
                                  EXECUTABLE UnitTest
                                  DEPENDENCIES UnitTest)
    endif()
endif()
