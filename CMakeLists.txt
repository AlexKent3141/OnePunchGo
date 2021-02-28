cmake_minimum_required (VERSION 3.16.3)
project(OnePunchGo)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

include_directories(
  "./src")

add_subdirectory(
  "./src")

if (MSVC)
  # TODO
else()
  add_compile_options(
    -Wall -Wextra -Wpedantic -O3)
endif()
