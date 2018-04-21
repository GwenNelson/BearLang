cmake_minimum_required(VERSION 3.0)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

set(CMAKE_C_FLAGS "-DREDIRECT_MALLOC=GC_malloc -DIGNORE_FREE")

include(CodeCoverage)

APPEND_COVERAGE_COMPILER_FLAGS()
find_package (LibGc REQUIRED)
find_package (Readline REQUIRED)
find_package (FLEX REQUIRED)

file(GLOB RUNTIME_SOURCE "runtime/src/*.c")

include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${LIBGC_INCLUDE_DIR})
include_directories(${Readline_INCLUDE_DIR})
include_directories(${FLEX_INCLUDES})

FLEX_TARGET(BLLexer
            "runtime/src/lexer.l"
            ${CMAKE_CURRENT_BINARY_DIR}/lexer.c)

add_library(blruntime SHARED ${RUNTIME_SOURCE} ${FLEX_BLLexer_OUTPUTS})
target_link_libraries(blruntime ${LIBGC_LIBRARIES} ${FLEX_LIBRARIES})

enable_testing()


file(GLOB RUNTIME_TEST_SRC "tests/runtime/*.c")
file(GLOB BLI_SRC          "bli/src/*.c")


add_executable(bli ${BLI_SRC})
target_link_libraries(bli blruntime)
target_link_libraries(bli ${Readline_LIBRARY})

APPEND_COVERAGE_COMPILER_FLAGS()

add_executable(test_blruntime ${RUNTIME_TEST_SRC})
target_link_libraries(test_blruntime blruntime)
add_test(NAME blruntime
         COMMAND test_blruntime)

set(COVERAGE_EXCLUDES "tests/*" "runtime/src/mpc.c")

SETUP_TARGET_FOR_COVERAGE(NAME test_coverage
                          EXECUTABLE test_blruntime
                          DEPENDENCIES test_blruntime)

