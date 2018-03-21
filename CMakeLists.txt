cmake_minimum_required(VERSION 3.0)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

file(GLOB RUNTIME_SOURCE "runtime/src/*.c")

include_directories(${CMAKE_SOURCE_DIR}/include)

add_library(blruntime SHARED ${RUNTIME_SOURCE})

enable_testing()

file(GLOB RUNTIME_TEST_SRC "tests/runtime/*.c")

add_executable(test_blruntime ${RUNTIME_TEST_SRC})
add_test(NAME blruntime
         COMMAND test_blruntime)
