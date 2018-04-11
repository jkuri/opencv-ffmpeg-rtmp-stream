cmake_minimum_required(VERSION 3.7)
project(rtmp-stream)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_SUPPRESS_REGENERATION true)

find_package(OpenCV REQUIRED)

set(INC_DIRS ${PROJECT_SOURCE_DIR}/include ${OpenCV_INCLUDE_DIRS})
set(LIBS ${OpenCV_LIBS} -lavcodec -lavformat -lavutil -lswscale -lavfilter -lavutil -lavdevice)

set(PROJECT_INCLUDE_DIRS ${INC_DIRS})

add_executable(rtmp-stream ${PROJECT_SOURCE_DIR}/src/rtmp-stream.cpp)

target_include_directories(rtmp-stream PUBLIC ${PROJECT_INCLUDE_DIRS})
target_link_libraries(rtmp-stream ${LIBS})
