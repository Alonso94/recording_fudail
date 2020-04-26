cmake_minimum_required(VERSION 2.8)
project(recording_video)

find_package(OpenCV REQUIRED)
add_executable( recording recording_video.cpp )
target_link_libraries( recording ${OpenCV_LIBS} )