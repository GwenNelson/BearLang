file(GLOB OS_SOURCE "src/*.c")

add_library(os SHARED ${OS_SOURCE})
target_link_libraries(os blruntime)
