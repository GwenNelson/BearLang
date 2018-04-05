cmake_minimum_required(VERSION 3.0)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

find_package (LibGc REQUIRED)
find_package (Readline REQUIRED)

file(GLOB RUNTIME_SOURCE "runtime/src/*.c")

include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${LIBGC_INCLUDE_DIR})
include_directories(${Readline_INCLUDE_DIR})

add_library(blruntime SHARED ${RUNTIME_SOURCE})
target_link_libraries(blruntime ${LIBGC_LIBRARIES})

enable_testing()

file(GLOB RUNTIME_TEST_SRC "tests/runtime/*.c")
file(GLOB BLI_SRC          "bli/src/*.c")

add_executable(test_blruntime ${RUNTIME_TEST_SRC})
target_link_libraries(test_blruntime blruntime)
add_test(NAME blruntime
         COMMAND test_blruntime)

add_executable(bli ${BLI_SRC})
target_link_libraries(bli blruntime)
target_link_libraries(bli ${Readline_LIBRARY})
