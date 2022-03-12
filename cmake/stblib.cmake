add_library(stb_image STATIC ${CMAKE_SOURCE_DIR}/3rdparty/stb_image.c)
target_include_directories(stb_image PUBLIC ${CMAKE_SOURCE_DIR}/3rdparty/)
