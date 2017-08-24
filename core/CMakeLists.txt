include_directories("include")

file(GLOB SRC "src/uber/jaeger/metrics/*.cpp"
              "src/uber/jaeger/utils/*.cpp")
list(FILTER SRC EXCLUDE REGEX ".*Test\\.cpp")
add_library(jaegercore ${SRC})

file(GLOB TEST_SRC "src/uber/jaeger/metrics/*Test.cpp"
                   "src/uber/jaeger/utils/*Test.cpp")
add_executable(CoreTest ${TEST_SRC})
target_link_libraries(CoreTest ${GTEST_BOTH_LIBRARIES} jaegercore)
add_test(NAME CoreTest
         COMMAND CoreTest)
if(CMAKE_BUILD_TYPE MATCHES Debug)
    setup_target_for_coverage(NAME CoreTestCoverage
                              EXECUTABLE CoreTest
                              DEPENDENCIES CoreTest)
endif()
