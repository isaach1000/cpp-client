cmake_minimum_required(VERSION 3.1)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

include(HunterGate)
HunterGate(
    URL "https://github.com/ruslo/hunter/archive/v0.19.125.tar.gz"
    SHA1 "371aef6239eff8639fe2cd94afef0f4d9f733302"
    FILEPATH "${CMAKE_SOURCE_DIR}/cmake/HunterConfig.cmake"
)

project(jaegertracing VERSION 0.0.1)

set(CMAKE_CXX_STANDARD_REQUIRED true)
set(CMAKE_CXX_STANDARD 11)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR
   CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -pedantic")
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-private-field")
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS true)

hunter_add_package(Boost)
find_package(Boost CONFIG REQUIRED)

hunter_add_package(thrift)
find_package(thrift CONFIG REQUIRED)

hunter_add_package(spdlog)
find_package(spdlog CONFIG REQUIRED)
list(APPEND LIBS spdlog::spdlog)

hunter_add_package(opentracing-cpp)
find_package(OpenTracing CONFIG REQUIRED)
list(APPEND LIBS OpenTracing::opentracing-static)

option(WITH_YAML_CPP "Use yaml-cpp to parse config files" true)
if(WITH_YAML_CPP)
    hunter_add_package(yaml-cpp)
    find_package(yaml-cpp CONFIG REQUIRED)
    list(APPEND LIBS yaml-cpp::yaml-cpp)
endif()

include(CTest)
if(BUILD_TESTING)
    hunter_add_package(GTest)
    find_package(GTest CONFIG REQUIRED)
    add_definitions(-DGTEST_HAS_TR1_TUPLE=0 -DGTEST_USE_OWN_TR1_TUPLE=0)

    if(CMAKE_BUILD_TYPE MATCHES Debug)
        include(CodeCoverage)
        APPEND_COVERAGE_COMPILER_FLAGS()
        set(COVERAGE_EXCLUDES "${CMAKE_SOURCE_DIR}/src/jaegertracing/thrift-gen/*"
                              "*Test.cpp")
    endif()
endif()

set(SUBDIRS baggage
            metrics
            net
            net/http
            propagation
            platform
            reporters
            samplers
            utils)

file(GLOB SRC "src/jaegertracing/*.cpp")
foreach(SUBDIR IN LISTS SUBDIRS)
    file(GLOB SUBDIR_SRC "src/jaegertracing/${SUBDIR}/*.cpp")
    list(FILTER SUBDIR_SRC EXCLUDE REGEX ".*Test\\.cpp")
    list(APPEND SRC ${SUBDIR_SRC})
endforeach()

add_subdirectory("src/jaegertracing/thrift-gen")
list(APPEND LIBS thriftgen thrift::thrift_static)

add_library(jaegertracing ${SRC})
set(JAEGERTRACING_WITH_YAML_CPP ${WITH_YAML_CPP})
configure_file(src/jaegertracing/Constants.h.in
               src/jaegertracing/Constants.h
               @ONLY)
target_include_directories(jaegertracing PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
    $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/src>)
target_link_libraries(jaegertracing ${LIBS})

if(BUILD_TESTING)
    add_subdirectory("${CMAKE_SOURCE_DIR}/src/jaegertracing/testutils")
    file(GLOB TEST_SRC "src/jaegertracing/*Test.cpp"
                       "src/jaegertracing/testutils/*Test.cpp")
    foreach(SUBDIR IN LISTS SUBDIRS)
        file(GLOB SUBDIR_TEST_SRC "src/jaegertracing/${SUBDIR}/*Test.cpp")
        list(APPEND TEST_SRC ${SUBDIR_TEST_SRC})
    endforeach()

    add_executable(UnitTest ${TEST_SRC})
    target_link_libraries(UnitTest testutils GTest::main jaegertracing ${LIBS})
    add_test(NAME UnitTest
             COMMAND UnitTest)
    if(CMAKE_BUILD_TYPE MATCHES Debug)
        setup_target_for_coverage(NAME UnitTestCoverage
                                  EXECUTABLE UnitTest
                                  DEPENDENCIES UnitTest)
    endif()
endif()
