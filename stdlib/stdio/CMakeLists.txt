file(GLOB STDIO_SOURCE "src/*.c")

add_library(stdio SHARED ${STDIO_SOURCE})
target_link_libraries(stdio blruntime)
