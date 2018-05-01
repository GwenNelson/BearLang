cmake_minimum_required(VERSION 3.0)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

add_compile_options("-DHASH_FUNCTION=HASH_OAT")
#add_compile_options("-O3")

include(CodeCoverage)

APPEND_COVERAGE_COMPILER_FLAGS()
find_package (LibGc REQUIRED)
find_package (Readline REQUIRED)
find_package (FLEX REQUIRED)
find_package (LLVM REQUIRED CONFIG)


file(GLOB RUNTIME_SOURCE "runtime/src/*.c")

include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${LIBGC_INCLUDE_DIR})
include_directories(${Readline_INCLUDE_DIR})
include_directories(${FLEX_INCLUDES})
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})


FLEX_TARGET(BLLexer
            "runtime/src/lexer.l"
            ${CMAKE_CURRENT_BINARY_DIR}/lexer.c
	    COMPILE_FLAGS "-v"
	    DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/bl_lexer.h)

add_library(blruntime SHARED ${RUNTIME_SOURCE} ${FLEX_BLLexer_OUTPUTS})
target_link_libraries(blruntime ${LIBGC_LIBRARIES} ${FLEX_LIBRARIES} dl gmp)

add_subdirectory(stdlib)

enable_testing()

file(GLOB RUNTIME_TEST_SRC "tests/runtime/*.c")
file(GLOB PARSER_TEST_SRC  "tests/parser/*.c")
file(GLOB BLI_SRC          "bli/src/*.c")
file(GLOB BLC_SRC          "blc/src/*.c")

add_executable(bli ${BLI_SRC})
add_executable(blc ${BLC_SRC})
target_link_libraries(bli blruntime)
target_link_libraries(bli ${Readline_LIBRARY})

llvm_map_components_to_libnames(llvm_libs core executionengine mcjit interpreter analysis native bitwriter)

target_link_libraries(blc blruntime)
target_link_libraries(blc ${llvm_libs})

APPEND_COVERAGE_COMPILER_FLAGS()

add_executable(test_blruntime ${RUNTIME_TEST_SRC})
target_link_libraries(test_blruntime blruntime)
add_test(NAME blruntime
         COMMAND test_blruntime)

add_executable(test_parser ${PARSER_TEST_SRC})
target_link_libraries(test_parser blruntime)

set(COVERAGE_EXCLUDES "tests/*" "runtime/src/mpc.c")

SETUP_TARGET_FOR_COVERAGE(NAME test_coverage
                          EXECUTABLE test_blruntime
                          DEPENDENCIES test_blruntime)

