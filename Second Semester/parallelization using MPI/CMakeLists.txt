cmake_minimum_required(VERSION 2.8.4)
project(parallelization_using_MPI)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.cpp)
add_executable(parallelization_using_MPI ${SOURCE_FILES})