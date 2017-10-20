set(SRC
    agent_constants.cpp
    Agent.cpp
    agent_types.cpp
    aggregation_validator_constants.cpp
    AggregationValidator.cpp
    aggregation_validator_types.cpp
    baggage_constants.cpp
    BaggageRestrictionManager.cpp
    baggage_types.cpp
    Collector.cpp
    dependency_constants.cpp
    Dependency.cpp
    dependency_types.cpp
    jaeger_constants.cpp
    jaeger_types.cpp
    sampling_constants.cpp
    SamplingManager.cpp
    sampling_types.cpp
    TracedService.cpp
    tracetest_constants.cpp
    tracetest_types.cpp
    ZipkinCollector.cpp
    zipkincore_constants.cpp
    zipkincore_types.cpp)
add_library(jaegertracing_thriftgen ${SRC})
target_link_libraries(jaegertracing_thriftgen thrift::thrift_static)
target_include_directories(jaegertracing_thriftgen PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src/jaegertracing/thrift-gen>)
