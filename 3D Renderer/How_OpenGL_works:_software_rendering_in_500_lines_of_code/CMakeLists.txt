cmake_minimum_required(VERSION 3.15)
project(Introduction_to_Ray_Tracing__a_Simple_Method_for_Creating_3D_Images)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "-O3 -Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g -Wall -Wextra")

include_directories(include)
add_executable(Lesson0 src/Lesson0.cpp src/tgaimage.cpp)
add_executable(Lesson1 src/Lesson1.cpp src/tgaimage.cpp src/model.cpp)
add_executable(Lesson2 src/Lesson2.cpp src/tgaimage.cpp src/model.cpp)