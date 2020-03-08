cmake_minimum_required(VERSION 3.1)
project(simpleNodes)

# where CMake modules are
# CMAKE_INSTALL_PREFIX tells install function where is the start pointof the path
# CMAKE_CURRENT_BINARY_DIR is where cmake is creating the visual studio project
set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_BINARY_DIR}/install)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/modules)
message(STATUS "Module path: ${CMAKE_MODULE_PATH}" )

# add folders where to reach CMakeLists.txt files
add_subdirectory(simple_transform)
