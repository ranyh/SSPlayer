cmake_minimum_required(VERSION 3.18.0)

project(Player VERSION 0.0.1)

set(CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS -std=c++11)
set(CMAKE_C_FLAGS -std=c99)

find_program(LN ln)
if (NOT LN-NOTFOUND)
exec_program(${LN} ${CMAKE_BINARY_DIR}
        ARGS -s ${CMAKE_SOURCE_DIR}/resources .)
endif()

include(${CMAKE_SOURCE_DIR}/cmake/glad.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/stblib.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/glm.cmake)

add_subdirectory(src)
