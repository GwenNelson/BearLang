file(GLOB SIMPLEWEB_SOURCE "src/*.c")

add_library(simpleweb SHARED ${SIMPLEWEB_SOURCE})
target_link_libraries(simpleweb blruntime)
