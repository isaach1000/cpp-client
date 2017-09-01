include_directories("${CMAKE_SOURCE_DIR}/include/uber/jaeger/thrift_gen")

file(GLOB SRC "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
list(FILTER SRC EXCLUDE REGEX ".*\\.skeleton\\.cpp")
add_library(jaegerthriftgen ${SRC})
target_link_libraries(jaegerthriftgen ${THRIFT_LIBS})
