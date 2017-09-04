file(GLOB SRC "*.cpp")
list(FILTER SRC EXCLUDE REGEX ".*Test\\.cpp")
add_library(testutils ${SRC})
