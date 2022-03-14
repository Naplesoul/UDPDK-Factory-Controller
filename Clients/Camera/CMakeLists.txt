cmake_minimum_required(VERSION 3.10)

project(Client-Camera)

add_executable(camera camera.cpp)

find_package(PkgConfig REQUIRED)
pkg_check_modules(dpdk REQUIRED IMPORTED_TARGET libdpdk)

target_link_libraries(camera PkgConfig::dpdk udpdk)