cmake_minimum_required(VERSION 2.8)
project(downloader)

set(CMAKE_CXX_FLAGS "-std=c++11")
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR})

link_directories(${PROJECT_SOURCE_DIR}/libs)
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
set(CMAKE_INSTALL_RPATH ${PROJECT_SOURCE_DIR}/libs)

aux_source_directory(. SRCS)
add_executable(${PROJECT_NAME} ${SRCS})
target_link_libraries(${PROJECT_NAME} curl pthread)