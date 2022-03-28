cmake_minimum_required(VERSION 3.10)

project(UDPDK-Server)

add_executable(server server.cpp Scheduler.cpp UDPServer.cpp Objects.cpp)

# find_package(PkgConfig REQUIRED)
# pkg_check_modules(dpdk REQUIRED IMPORTED_TARGET libdpdk)

# target_link_libraries(server PkgConfig::dpdk udpdk jsoncpp pthread)
target_link_libraries(server jsoncpp pthread)