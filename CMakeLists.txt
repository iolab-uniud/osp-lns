cmake_minimum_required(VERSION 3.14)

project(osp CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
list(INSERT CMAKE_MODULE_PATH 0 ${PROJECT_SOURCE_DIR}/easylocal-3/cmake)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)

include_directories(${PROJECT_SOURCE_DIR}/local-search)

# find_package(Boost 1.71 COMPONENTS date_time)

add_subdirectory(easylocal-3)
add_subdirectory(local-search)
