cmake_minimum_required(VERSION 3.10)
project(json_subgraph_extractor)

set(CMAKE_CXX_STANDARD 17)

include_directories(libs)

add_executable(mytool main.cpp)
