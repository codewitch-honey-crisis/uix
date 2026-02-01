cmake_minimum_required(VERSION 3.24)
if(NOT DEFINED IDF_TARGET)
    project(htcw_uix VERSION 1.8.14)
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED True)
    set(CMAKE_STATIC_LIBRARY_PREFIX "")
    set(CMAKE_SHARED_LIBRARY_PREFIX "")

    include(FetchContent)
    FetchContent_Declare(htcw_gfx
        GIT_REPOSITORY https://github.com/codewitch-honey-crisis/gfx.git
        GIT_TAG 2.2.16
        GIT_SHALLOW ON
        FIND_PACKAGE_ARGS 2.2.16
    )

    FetchContent_MakeAvailable(htcw_gfx)

    add_library(htcw_uix src/source/uix_display.cpp)

    target_link_libraries(htcw_uix htcw_gfx htcw_bits htcw_io htcw_data htcw_ml)

    target_include_directories(htcw_uix PUBLIC
    "${PROJECT_SOURCE_DIR}"
    "${PROJECT_SOURCE_DIR}/src"
    "${PROJECT_BINARY_DIR}")
else()
    idf_component_register(
        SRCS "./src/source/uix_display.cpp"
        INCLUDE_DIRS "." "./src"
        REQUIRES htcw_gfx
    )
endif()