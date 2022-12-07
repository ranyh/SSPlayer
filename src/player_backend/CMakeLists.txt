project(player_backend)

set(PLAYER_BACKEND_SRC
        player.h
        player.cpp
        backend.h
        backend.cpp
        frame.cpp
    )

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

if (WITH_GSTREAMER)
    set(PLAYER_BACKEND_SRC
            ${PLAYER_BACKEND_SRC}
            backend/gst.h
            backend/gst.cpp)
    pkg_check_modules(GST REQUIRED gstreamer-1.0)
    pkg_check_modules(GST_APP REQUIRED gstreamer-app-1.0)
    pkg_check_modules(GST_PBUTILS REQUIRED gstreamer-pbutils-1.0)
    pkg_check_modules(GST_ALLOCATORS REQUIRED gstreamer-allocators-1.0)

    include_directories(
            ${GST_INCLUDE_DIRS}
            ${GST_APP_INCLUDE_DIRS})

    set(LINK_LIBRARIES
            ${GST_LIBRARIES}
            ${GST_APP_LIBRARIES}
            ${GST_PBUTILS_LIBRARIES}
            ${GST_ALLOCATORS_LIBRARIES})


endif()

if (WITH_FFMPEG)
    set(PLAYER_BACKEND_SRC
            ${PLAYER_BACKEND_SRC}
            backend/ffmpeg.h
            backend/ffmpeg.cpp)

    pkg_check_modules(AVCODEC libavcodec REQUIRED)
    pkg_check_modules(AVUTIL libavutil REQUIRED)
    pkg_check_modules(AVFORMAT libavformat REQUIRED)
    pkg_check_modules(SWRESAMPLE libswresample REQUIRED)
    pkg_check_modules(SWSCALE libswscale REQUIRED)
    pkg_check_modules(AVFILTER libavfilter REQUIRED)

    include_directories(
            ${AVCODEC_INCLUDE_DIRS}
            ${AVUTIL_INCLUDE_DIRS}
            ${AVFORMAT_INCLUDE_DIRS}
            ${AVFILTER_INCLUDE_DIRS})

    set(LINK_LIBRARIES
            ${LINK_LIBRARIES}
            ${AVCODEC_LIBRARIES}
            ${AVFILTER_LIBRARIES}
            ${AVUTIL_LIBRARIES}
            ${AVFORMAT_LIBRARIES}
            ${SWRESAMPLE_LIBRARIES}
            ${SWSCALE_LIBRARIES})
endif()

add_library(${PROJECT_NAME} STATIC ${PLAYER_BACKEND_SRC})
target_link_libraries(${PROJECT_NAME} INTERFACE ${LINK_LIBRARIES}
                PRIVATE spdlog::spdlog)
target_include_directories(${PROJECT_NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/)


enable_testing()
add_executable(frame_pool_test
        frame_pool_test.cpp)
target_link_libraries(frame_pool_test
        GTest::gtest_main)
if (NOT CMAKE_CROSSCOMPILING)
include(GoogleTest)
gtest_discover_tests(frame_pool_test)
endif()
