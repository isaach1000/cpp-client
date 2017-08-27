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

add_library(jaegercore ${SRC})

add_executable(CoreTest ${TEST_SRC})
target_link_libraries(CoreTest ${GTEST_BOTH_LIBRARIES} jaegercore)
add_test(NAME CoreTest
         COMMAND CoreTest)
if(CMAKE_BUILD_TYPE MATCHES Debug)
    setup_target_for_coverage(NAME CoreTestCoverage
                              EXECUTABLE CoreTest
                              DEPENDENCIES CoreTest)
endif()
