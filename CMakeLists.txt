cmake_minimum_required(VERSION 3.1)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

include(HunterGate)
HunterGate(
    URL "https://github.com/ruslo/hunter/archive/v0.19.127.tar.gz"
    SHA1 "d685a47797ab9fd724b910c67505765cf1fe402a"
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

add_subdirectory("src/jaegertracing/thrift-gen")
list(APPEND LIBS jaegertracing_thriftgen thrift::thrift_static)

set(SRC
    src/jaegertracing/TraceID.cpp
    src/jaegertracing/utils/RateLimiter.cpp
    src/jaegertracing/utils/ErrorUtil.cpp
    src/jaegertracing/utils/HexParsing.cpp
    src/jaegertracing/utils/YAML.cpp
    src/jaegertracing/utils/UDPClient.cpp
    src/jaegertracing/reporters/InMemoryReporter.cpp
    src/jaegertracing/reporters/Config.cpp
    src/jaegertracing/reporters/NullReporter.cpp
    src/jaegertracing/reporters/CompositeReporter.cpp
    src/jaegertracing/reporters/LoggingReporter.cpp
    src/jaegertracing/reporters/Reporter.cpp
    src/jaegertracing/reporters/RemoteReporter.cpp
    src/jaegertracing/LogRecord.cpp
    src/jaegertracing/propagation/HeadersConfig.cpp
    src/jaegertracing/propagation/Injector.cpp
    src/jaegertracing/propagation/Propagator.cpp
    src/jaegertracing/propagation/Extractor.cpp
    src/jaegertracing/platform/Endian.cpp
    src/jaegertracing/platform/Hostname.cpp
    src/jaegertracing/UDPTransport.cpp
    src/jaegertracing/metrics/Gauge.cpp
    src/jaegertracing/metrics/Metrics.cpp
    src/jaegertracing/metrics/NullStatsReporter.cpp
    src/jaegertracing/metrics/StatsFactoryImpl.cpp
    src/jaegertracing/metrics/StatsReporter.cpp
    src/jaegertracing/metrics/StatsFactory.cpp
    src/jaegertracing/metrics/NullGauge.cpp
    src/jaegertracing/metrics/Metric.cpp
    src/jaegertracing/metrics/NullStatsFactory.cpp
    src/jaegertracing/metrics/NullCounter.cpp
    src/jaegertracing/metrics/Timer.cpp
    src/jaegertracing/metrics/InMemoryStatsReporter.cpp
    src/jaegertracing/metrics/Counter.cpp
    src/jaegertracing/metrics/NullTimer.cpp
    src/jaegertracing/samplers/SamplingStatus.cpp
    src/jaegertracing/samplers/RateLimitingSampler.cpp
    src/jaegertracing/samplers/ConstSampler.cpp
    src/jaegertracing/samplers/ProbabilisticSampler.cpp
    src/jaegertracing/samplers/Config.cpp
    src/jaegertracing/samplers/AdaptiveSampler.cpp
    src/jaegertracing/samplers/GuaranteedThroughputProbabilisticSampler.cpp
    src/jaegertracing/samplers/RemotelyControlledSampler.cpp
    src/jaegertracing/samplers/Sampler.cpp
    src/jaegertracing/Config.cpp
    src/jaegertracing/net/IPAddress.cpp
    src/jaegertracing/net/http/Header.cpp
    src/jaegertracing/net/http/Response.cpp
    src/jaegertracing/net/http/Request.cpp
    src/jaegertracing/net/http/Method.cpp
    src/jaegertracing/net/http/Error.cpp
    src/jaegertracing/net/URI.cpp
    src/jaegertracing/net/Socket.cpp
    src/jaegertracing/Reference.cpp
    src/jaegertracing/SpanContext.cpp
    src/jaegertracing/Span.cpp
    src/jaegertracing/Tag.cpp
    src/jaegertracing/Logging.cpp
    src/jaegertracing/baggage/RestrictionsConfig.cpp
    src/jaegertracing/baggage/Restriction.cpp
    src/jaegertracing/baggage/RestrictionManager.cpp
    src/jaegertracing/baggage/BaggageSetter.cpp
    src/jaegertracing/Tracer.cpp
    src/jaegertracing/Transport.cpp)
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
    set(TEST_SRC
        src/jaegertracing/utils/UDPClientTest.cpp
        src/jaegertracing/utils/RateLimiterTest.cpp
        src/jaegertracing/reporters/ReporterTest.cpp
        src/jaegertracing/SpanContextTest.cpp
        src/jaegertracing/propagation/PropagatorTest.cpp
        src/jaegertracing/metrics/NullStatsFactoryTest.cpp
        src/jaegertracing/metrics/MetricsTest.cpp
        src/jaegertracing/samplers/SamplerTest.cpp
        src/jaegertracing/net/SocketTest.cpp
        src/jaegertracing/net/http/MethodTest.cpp
        src/jaegertracing/net/http/HeaderTest.cpp
        src/jaegertracing/net/http/ResponseTest.cpp
        src/jaegertracing/net/IPAddressTest.cpp
        src/jaegertracing/net/URITest.cpp
        src/jaegertracing/TracerTest.cpp
        src/jaegertracing/TraceIDTest.cpp
        src/jaegertracing/testutils/TUDPTransportTest.cpp
        src/jaegertracing/testutils/MockAgentTest.cpp
        src/jaegertracing/ConfigTest.cpp
        src/jaegertracing/TagTest.cpp)
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
