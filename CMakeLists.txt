cmake_minimum_required(VERSION 3.24)
project(htcw_uix VERSION 1.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_STATIC_LIBRARY_PREFIX "")
set(CMAKE_SHARED_LIBRARY_PREFIX "")

add_library(htcw_uix src/uix_display.cpp)
target_include_directories(htcw_uix INTERFACE
    "${PROJECT_SOURCE_DIR}/include"
    "${PROJECT_BINARY_DIR}"
)
target_link_libraries(htcw_uix htcw_gfx htcw_bits htcw_io htcw_data htcw_ml)

target_include_directories(htcw_uix PUBLIC
"${PROJECT_SOURCE_DIR}/include"
"${PROJECT_SOURCE_DIR}/src"
"${PROJECT_BINARY_DIR}")
