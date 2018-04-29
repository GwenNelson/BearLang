file(GLOB HELLO_SOURCE "src/*.c")

add_library(hello SHARED ${HELLO_SOURCE})
target_link_libraries(hello blruntime)
