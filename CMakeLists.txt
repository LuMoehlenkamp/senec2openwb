cmake_minimum_required(VERSION 3.25 FATAL_ERROR)
set(CMAKE_SYSTEM_NAME Linux)
set(PROJECT_NAME_STR senec2openwb)
project(${PROJECT_NAME_STR} VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_GENERATOR "Ninja")
set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_C_COMPILER "gcc")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# set(CMAKE_CXX_SCAN_FOR_MODULES TRUE)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")
# include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -ldl -Wall -Wextra -Wshadow -Wnon-virtual-dtor -Wold-style-cast -Wcast-align -Wunused -Woverloaded-virtual -Wpedantic -Wconversion -Wsign-conversion -Wnull-dereference -Wdouble-promotion -Wformat=2 -Wduplicated-cond -Wduplicated-branches -Wlogical-op -Wuseless-cast")
file(GLOB_RECURSE INC_FILES src/*.hh)
file(GLOB_RECURSE SRC_FILES src/*.cc)
# file(GLOB_RECURSE CXX_MODULES src/*.ccm)

add_subdirectory(src)

# testing
if( CMAKE_BUILD_TYPE MATCHES Debug )
  include(CTest)
  add_subdirectory(test)
endif()

include_directories(${Boost_INCLUDE_DIRS})