cmake_minimum_required (VERSION 3.16.3)
project(OnePunchGo)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

SET(PROFILE 0 CACHE BOOL "Configure the Lurien profiler")

if (PROFILE)
  add_compile_definitions(LURIEN_ENABLED=1)
else()
  add_compile_definitions(LURIEN_ENABLED=0)
endif()

if (MSVC)
  set(CMAKE_CXX_FLAGS_DEBUG "/std:c++latest")
  set(CMAKE_CXX_FLAGS_RELEASE "/std:c++latest")
else()
  set(CMAKE_CXX_FLAGS_DEBUG "-Wall -Wextra -Wpedantic -std=c++2a -O0 -g")
  set(CMAKE_CXX_FLAGS_RELEASE "-Wall -Wextra -Wpedantic -std=c++2a -O3")
endif()

include_directories(
  "./src")

add_subdirectory(
  "./src")
