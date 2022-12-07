cmake_minimum_required(VERSION 3.18.0)

project(Player VERSION 0.1.0)

set(CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")

option(WITH_GSTREAMER "Build with GStreamer" OFF)
option(WITH_FFMPEG "Build with FFmpeg" ON)

if (WITH_GSTREAMER)
    add_definitions(-DGST_BACKEND)
endif()

if (WITH_FFMPEG)
    add_definitions(-DFFMPEG_BACKEND)
endif()

file(CREATE_LINK ${CMAKE_SOURCE_DIR}/resources ${CMAKE_BINARY_DIR}/resources SYMBOLIC)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/58d77fa8070e8cec2dc1ed015d66b454c8d78850.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

include(${CMAKE_SOURCE_DIR}/cmake/glad.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/stblib.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/glm.cmake)

add_subdirectory(3rdparty/spdlog)
add_subdirectory(src)
