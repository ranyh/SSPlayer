set(SRC main.cpp
    shader.cpp)

find_package(PkgConfig REQUIRED)
pkg_check_modules(FREE_TYPE2 freetype2 REQUIRED)
pkg_check_modules(SDL2 sdl2)

include_directories(${SDL2_INCLUDE_DIRS})

add_executable(player ${SRC})
target_link_libraries(player glad ${SDL2_LINK_LIBRARIES})
