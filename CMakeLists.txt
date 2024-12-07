cmake_minimum_required(VERSION 3.10)
project(Project)

set(CMAKE_CXX_STANDARD 20)

add_executable(Project main.cpp )


find_package(ZLIB REQUIRED)
include_directories(${ZLIB_INCLUDE_DIRS})
target_link_libraries(Project PRIVATE ZLIB::ZLIB)

find_package(unofficial-minizip CONFIG REQUIRED)
target_link_libraries(Project PRIVATE unofficial::minizip::minizip)

find_package(libzip CONFIG REQUIRED)
target_link_libraries(Project PRIVATE libzip::zip)