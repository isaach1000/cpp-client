include_directories("include")

file(GLOB SRC "src/uber/jaeger/metrics/*.cpp")
list(FILTER SRC EXCLUDE REGEX ".*Test\\.cpp")
add_library(jaegercore ${SRC})

file(GLOB TEST_SRC "src/uber/jaeger/metrics/*Test.cpp")
add_executable(MetricsTest ${TEST_SRC})
target_link_libraries(MetricsTest ${GTEST_BOTH_LIBRARIES} jaegercore)
add_test(MetricsTest MetricsTest)
