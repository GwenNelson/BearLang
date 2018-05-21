file(GLOB XML_SOURCE "src/*.c")

add_library(xml SHARED ${XML_SOURCE})
target_link_libraries(xml blruntime)
