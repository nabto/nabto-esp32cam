# The following lines of boilerplate have to be in your project's CMakeLists
# in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)

set(UNABTO_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/unabto)
include(./unabto/build/cmake/unabto_files.cmake)

set(ENV{EXTRA_CFLAGS} -Wno-maybe-uninitialized) 

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(esp32_nabto)

set(COMPONENTS esp32-camera nabtotunnel unabto-esp-fingerprint unabto-esp-idf)

