cmake_minimum_required(VERSION 3.10)
project(json-extractor CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (NOT CMAKE_BUILD_TYPE)
    # 默认使用Debug模式，方便开发调试
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build (Debug or Release)." FORCE)
endif()

add_executable(tool 
    src/main.cpp
)
add_library(Lib
    src/corealgor.cpp
    src/IOprocess.cpp    
)


target_include_directories(Lib PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)
target_link_libraries(tool Lib)
