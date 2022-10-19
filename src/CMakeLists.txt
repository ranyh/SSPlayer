set(SRC main.cpp
    application.cpp
    video_player.cpp
    sdl_event.cpp
    eventloop/event_loop.cpp
    eventloop/select_event_loop.cpp
    eventloop/event_loop_factory.cpp
    player/player.h
    player/player.cpp
    player/backend/backend.h
    player/backend/backend.cpp
    player/backend/frame.cpp
    player/backend/gst.h
    player/backend/gst.cpp
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
    )

find_package(PkgConfig REQUIRED)
pkg_check_modules(FREE_TYPE2 freetype2 REQUIRED)
pkg_check_modules(SDL2 sdl2)
pkg_check_modules(GST REQUIRED gstreamer-1.0)
pkg_check_modules(GST_APP REQUIRED gstreamer-app-1.0)
pkg_check_modules(GST_PBUTILS REQUIRED gstreamer-pbutils-1.0)
pkg_check_modules(GST_ALLOCATORS REQUIRED gstreamer-allocators-1.0)

include_directories(${SDL2_INCLUDE_DIRS}
    ${GST_INCLUDE_DIRS}
    ${GST_APP_INCLUDE_DIRS}
    ${FREE_TYPE2_INCLUDE_DIRS})

add_executable(player ${SRC})
target_link_libraries(player stb_image glad
        ${SDL2_LIBRARIES} ${GST_LIBRARIES} ${GST_APP_LIBRARIES}
        ${FREE_TYPE2_LIBRARIES} ${GST_PBUTILS_LIBRARIES} ${GST_ALLOCATORS_LIBRARIES})
