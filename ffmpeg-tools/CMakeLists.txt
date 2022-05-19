cmake_minimum_required(VERSION 3.22)
project(ffmpeg_tools C)

set(CMAKE_C_STANDARD 17)

add_compile_definitions(_USING_TRACE_MODE_)

include_directories(/usr/local/include)
link_directories(/usr/lib /usr/local/lib)

add_executable(ffmpeg_tools main.c common.c  transcoding.c log.h log.c)

target_link_libraries(ffmpeg_tools avformat avcodec avutil avfilter)

