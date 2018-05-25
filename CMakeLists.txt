cmake_minimum_required(VERSION 3.7)
project(TinyRayTracingRender)

set(CMAKE_CXX_STANDARD 11)
find_package( OpenCV REQUIRED )
set(SOURCE_FILES main.cpp)
add_executable(TinyRayTracingRender ${SOURCE_FILES})
target_link_libraries( TinyRayTracingRender ${OpenCV_LIBS} )