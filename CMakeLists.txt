cmake_minimum_required(VERSION 3.1)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

include(HunterGate)
HunterGate(
    URL "https://github.com/ruslo/hunter/archive/v0.19.115.tar.gz"
    SHA1 "aeb37c4b667c4201837bcac38702f3ce8b4a9b44")

project(jaegerclient)

set(CMAKE_CXX_STANDARD_REQUIRED true)
set(CMAKE_CXX_STANDARD 11)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR
   CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -pedantic -Wno-unused-private-field")
    set(CMAKE_EXPORT_COMPILE_COMMANDS true)
endif()

hunter_add_package(Boost)
find_package(Boost CONFIG REQUIRED)

hunter_add_package(thrift)
find_package(thrift CONFIG REQUIRED)

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


set(SUBDIRS metrics net propagation reporters samplers utils)

file(GLOB SRC "src/uber/jaeger/*.cpp")
foreach(SUBDIR IN LISTS SUBDIRS)
    file(GLOB SUBDIR_SRC "src/uber/jaeger/${SUBDIR}/*.cpp")
    list(FILTER SUBDIR_SRC EXCLUDE REGEX ".*Test\\.cpp")
    list(APPEND SRC ${SUBDIR_SRC})
endforeach()

add_subdirectory("src/uber/jaeger/thrift-gen")
list(APPEND LIBS thriftgen thrift::thrift_static)

add_library(jaeger ${SRC})
target_include_directories(jaeger PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>)
target_link_libraries(jaeger ${LIBS})

if(BUILD_TESTING)
    add_subdirectory("${CMAKE_SOURCE_DIR}/src/uber/jaeger/testutils")
    file(GLOB TEST_SRC "src/uber/jaeger/*Test.cpp"
                       "src/uber/jaeger/testutils/*Test.cpp")
    foreach(SUBDIR IN LISTS SUBDIRS)
        file(GLOB SUBDIR_TEST_SRC "src/uber/jaeger/${SUBDIR}/*Test.cpp")
        list(APPEND TEST_SRC ${SUBDIR_TEST_SRC})
    endforeach()

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
