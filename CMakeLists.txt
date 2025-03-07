cmake_minimum_required(VERSION 3.15.0 FATAL_ERROR)
set(CMAKE_SYSTEM_NAME Linux)
set(PROJECT_NAME_STR senec2openwb)
set(CMAKE_GENERATOR "Ninja")
project(${PROJECT_NAME_STR} VERSION 0.1.0 LANGUAGES CXX)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -ldl -Wall -Wextra -Wshadow -Wnon-virtual-dtor -Wold-style-cast -Wcast-align -Wunused -Woverloaded-virtual -Wpedantic -Wconversion -Wsign-conversion -Wnull-dereference -Wdouble-promotion -Wformat=2 -Wduplicated-cond -Wduplicated-branches -Wlogical-op -Wuseless-cast")
file(GLOB_RECURSE INC_FILES src/*.hh)
file(GLOB_RECURSE SRC_FILES src/*.cc)

add_subdirectory(src)

# testing
include(CTest)
add_subdirectory(test)

include_directories(${Boost_INCLUDE_DIRS})