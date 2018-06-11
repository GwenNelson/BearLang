file(GLOB TESTMOD2_SOURCE "src/*.c")

add_library(testmod2 SHARED ${TESTMOD2_SOURCE})
target_link_libraries(testmod2 blruntime)
