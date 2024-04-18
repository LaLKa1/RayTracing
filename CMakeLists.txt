cmake_minimum_required(VERSION 3.14)
project(rayshit)

set(CMAKE_CXX_STANDARD 17)
set(OUTPUT_NAME ray$hit.out)

find_package(SFML 2.5 COMPONENTS graphics REQUIRED)

add_executable(rayshit main.cpp)

target_link_libraries(rayshit sfml-graphics)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Ofast -fno-fast-math")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -fno-omit-frame-pointer ")
