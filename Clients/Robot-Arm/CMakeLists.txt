cmake_minimum_required(VERSION 3.10)

project(Client-Arm)

add_executable(arm camera.cpp)

find_package(PkgConfig REQUIRED)
pkg_check_modules(dpdk REQUIRED IMPORTED_TARGET libdpdk)

target_link_libraries(arm PkgConfig::dpdk udpdk)