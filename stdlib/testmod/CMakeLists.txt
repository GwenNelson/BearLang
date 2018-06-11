file(GLOB TESTMOD_SOURCE "src/*.c")

add_library(testmod SHARED ${TESTMOD_SOURCE})
target_link_libraries(testmod blruntime)
