cmake_minimum_required(VERSION 3.10)

project(Client-Camera)

add_executable(camera camera.cpp BlockTracker.cpp)

# find_package(PkgConfig REQUIRED)
# pkg_check_modules(dpdk REQUIRED IMPORTED_TARGET libdpdk)
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

target_link_libraries(camera ${OpenCV_LIBS})