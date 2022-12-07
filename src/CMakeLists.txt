set(SRC main.cpp
    application.cpp
    video_player.cpp
    sdl_event.cpp
    eventloop/event_loop.cpp
    eventloop/select_event_loop.cpp
    eventloop/event_loop_factory.cpp
    ui/director.cpp
    ui/ui_context.cpp
    ui/group.cpp
    ui/layer.cpp
    ui/scene.cpp
    ui/controller.cpp
    ui/text.cpp
    ui/icon.cpp
    ui/primitive.cpp
    ui/button.cpp
    ui/icon_button.cpp
    ui/slider.cpp
    ui/video_view.cpp
    ui/playlist.cpp
    ui/font/font.cpp
    ui/font/font_manager.cpp
    ui/gl/shader.cpp
    ui/gl/shader_manager.cpp
    ui/gl/texture.cpp
    ui/gl/buffers.cpp
    utils/time.cpp
    sound/audio_sink.cpp
    )

find_package(PkgConfig REQUIRED)
pkg_check_modules(FREE_TYPE2 freetype2 REQUIRED)
pkg_check_modules(SDL2 sdl2)
pkg_check_modules(EGL egl)

include_directories(
        ${SDL2_INCLUDE_DIRS}
        ${EGL_INCLUDE_DIRS}
        ${FREE_TYPE2_INCLUDE_DIRS}
        ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(player_backend)

add_executable(player ${SRC})
target_link_libraries(player PRIVATE stb_image glad player_backend spdlog::spdlog_header_only
        ${SDL2_LIBRARIES} ${FREE_TYPE2_LIBRARIES} ${EGL_LIBRARIES})
target_link_directories(player PRIVATE ${SDL2_LIBRARY_DIRS})
