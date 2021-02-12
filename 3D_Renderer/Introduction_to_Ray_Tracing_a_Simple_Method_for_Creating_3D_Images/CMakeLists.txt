cmake_minimum_required(VERSION 3.15)
project(Introduction_to_Ray_Tracing__a_Simple_Method_for_Creating_3D_Images)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "-O3 -Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g -Wall -Wextra")

add_executable(raytracer raytracer.cpp)
add_executable(minray minray.cpp)