file(GLOB TEST_SRC "uber/jaeger/metrics/*.cpp")

add_executable(metrics_test ${TEST_SRC})
target_link_libraries(metrics_test GTest::GTest GTest::Main jaegercore)

add_test(MetricsTest metrics_test)
