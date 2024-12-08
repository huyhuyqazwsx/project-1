cmake_minimum_required(VERSION 3.10)
project(Project)

set(CMAKE_CXX_STANDARD 20)

add_executable(Project main.cpp )

#duong dan o muc setting-> built -> cmake option them 2 duong dan nay khi clone ve may
#-DCMAKE_TOOLCHAIN_FILE=C:\Users\HUY\.vcpkg-clion\vcpkg\scripts\buildsystems\vcpkg.cmake
#-DVCPKG_TARGET_TRIPLET=x64-mingw-dynamic


find_package(ZLIB REQUIRED)
include_directories(${ZLIB_INCLUDE_DIRS})
target_link_libraries(Project PRIVATE ZLIB::ZLIB)

find_package(unofficial-minizip CONFIG REQUIRED)
target_link_libraries(Project PRIVATE unofficial::minizip::minizip)

find_package(libzip CONFIG REQUIRED)
target_link_libraries(Project PRIVATE libzip::zip)

find_package(7zip CONFIG REQUIRED)
target_link_libraries(Project PRIVATE 7zip::7zip)