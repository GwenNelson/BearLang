file(GLOB PY27_SOURCE "src/*.c")

find_package(PythonLibs 2.7 REQUIRED)
include_directories(${PYTHON_INCLUDE_DIRS})
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${PYFLAGS}")
add_library(py27 SHARED ${PY27_SOURCE})
target_link_libraries(py27 blruntime)
target_link_libraries(py27 ${PYTHON_LIBRARIES})
