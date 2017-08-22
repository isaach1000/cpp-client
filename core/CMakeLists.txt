include_directories("include")

file(GLOB SRC "src/uber/jaeger/metrics/*.cpp")
add_library(jaegercore ${SRC})
