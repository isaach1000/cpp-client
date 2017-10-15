file(GLOB SRC "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
list(FILTER SRC EXCLUDE REGEX ".*\\.skeleton\\.cpp")
add_library(thriftgen ${SRC})
target_link_libraries(thriftgen thrift::thrift_static)
target_include_directories(thriftgen PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src/jaegertracing/thrift-gen>)
