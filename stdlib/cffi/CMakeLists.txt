file(GLOB CFFI_SOURCE "src/*.c")

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBFFI REQUIRED libffi)

add_library(cffi SHARED ${CFFI_SOURCE})
include_directories(${LIBFFI_INCLUDE_DIRS})
target_link_libraries(cffi blruntime)
target_link_libraries(cffi ${LIBFFI_LIBRARIES})
