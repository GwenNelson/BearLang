file(GLOB ERROR_SOURCE "src/*.c")

add_library(error SHARED ${ERROR_SOURCE})
target_link_libraries(error blruntime)
