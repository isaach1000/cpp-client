include_directories("${CMAKE_SOURCE_DIR}/include/uber/jaeger/thrift_gen")

file(GLOB SRC "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
list(FILTER SRC EXCLUDE REGEX ".*\\.skeleton\\.cpp")
add_library(thrift_gen ${SRC})
target_link_libraries(thrift_gen ${THRIFT_LIBS})
