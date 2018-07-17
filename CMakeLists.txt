cmake_minimum_required(VERSION 3.0)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")
add_compile_options("-DHASH_FUNCTION=HASH_OAT")

if (CMAKE_BUILD_TYPE STREQUAL "Release")
	add_compile_options("-ffast-math")
	add_compile_options("-funroll-loops")
	add_compile_options("-march=native")
endif (CMAKE_BUILD_TYPE STREQUAL "Release")

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
	add_compile_options("-DHASH_DEBUG=1")
	add_compile_options("-DDEBUG")
	include(CodeCoverage)
	APPEND_COVERAGE_COMPILER_FLAGS()
endif  (CMAKE_BUILD_TYPE STREQUAL "Debug")

find_package (LibGc REQUIRED)
find_package (Readline REQUIRED)
find_package (FLEX REQUIRED)


file(GLOB RUNTIME_SOURCE "runtime/src/*.c")

set(VERSION_STR  "0.WHATEVER")
set(STDLIB_PATH1 /usr/lib/bearlang/${VERSION_STR})
set(STDLIB_PATH2 ${CMAKE_SOURCE_DIR}/stdlib)
set(STDLIB_PATH3 ${CMAKE_CURRENT_BINARY_DIR}/stdlib)

configure_file(${CMAKE_SOURCE_DIR}/include/bearlang/build_config.h.in bl_build_config.h @ONLY)

include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${LIBGC_INCLUDE_DIR})
include_directories(${Readline_INCLUDE_DIR})
include_directories(${FLEX_INCLUDES})
include_directories(${CMAKE_CURRENT_BINARY_DIR})


FLEX_TARGET(BLLexer
            "runtime/src/lexer.l"
            ${CMAKE_CURRENT_BINARY_DIR}/lexer.c
	    COMPILE_FLAGS "-v"
	    DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/bl_lexer.h)

add_library(blruntime SHARED ${RUNTIME_SOURCE} ${FLEX_BLLexer_OUTPUTS})
target_link_libraries(blruntime ${LIBGC_LIBRARIES} ${FLEX_LIBRARIES} ${Readline_LIBRARY} dl gmp)

add_subdirectory(stdlib)

enable_testing()

file(GLOB RUNTIME_TEST_SRC "tests/runtime/*.c")
file(GLOB PARSER_TEST_SRC  "tests/parser/*.c")
file(GLOB BLI_SRC          "bli/src/*.c")
file(GLOB BLC_SRC          "blc/src/*.c")

add_executable(bli ${BLI_SRC})
#add_executable(blc ${BLC_SRC})
target_link_libraries(bli blruntime)
target_link_libraries(bli ${Readline_LIBRARY})


#target_link_libraries(blc blruntime)
#target_link_libraries(blc ${llvm_libs})


add_executable(test_blruntime ${RUNTIME_TEST_SRC})
target_link_libraries(test_blruntime blruntime)
add_test(NAME blruntime
         COMMAND test_blruntime)

add_executable(test_parser ${PARSER_TEST_SRC})
target_link_libraries(test_parser blruntime)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
	set(COVERAGE_EXCLUDES ${CMAKE_CURRENT_BINARY_DIR} )

	SETUP_TARGET_FOR_COVERAGE(NAME test_coverage
                          EXECUTABLE test_blruntime
                          DEPENDENCIES test_blruntime)

endif  (CMAKE_BUILD_TYPE STREQUAL "Debug")

