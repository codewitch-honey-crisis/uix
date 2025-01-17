# Copyright (c) 2024 honey the codewitch
#
# SPDX-License-Identifier: Apache-2.0

if(CONFIG_HTCW_UIX)
zephyr_library()
#include_directories(${ZEPHYR_BASE}/lib/???/htcw_bits)
zephyr_include_directories(../src/)
zephyr_library_include_directories(../src/)
zephyr_interface_library_named(HTCW_UIX)
# zephyr_library_sources_ifdef(CONFIG_PARAM_DEF, file.cpp)
zephyr_library_sources(../src/source/uix_display.cpp)
    
#zephyr_library_sources_ifdef(CONFIG_PARAM_DEF, file.cpp)
#zephyr_library_sources()
zephyr_library_link_libraries(HTCW_UIX)
target_link_libraries(HTCW_UIX INTERFACE zephyr_interface)

endif()
