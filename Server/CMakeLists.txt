cmake_minimum_required(VERSION 3.10)

project(UDPDK-Server)

add_executable(server server.cpp Scheduler.cpp UDPServer.cpp Objects.cpp dpdk.cpp)

find_package(PkgConfig REQUIRED)
pkg_check_modules(odphelper REQUIRED IMPORTED_TARGET libodphelper)
pkg_check_modules(odp REQUIRED IMPORTED_TARGET libodp-linux)
pkg_check_modules(ofp REQUIRED IMPORTED_TARGET libofp)

target_link_libraries(server PkgConfig::ofp PkgConfig::odp PkgConfig::odphelper jsoncpp pthread)