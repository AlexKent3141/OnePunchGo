cmake_minimum_required (VERSION 3.16.3)
project(OnePunchGo)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

if (MSVC)
  # TODO
else()
  set(CMAKE_CXX_FLAGS_DEBUG "-Wall -Wextra -Wpedantic -O0 -g")
  set(CMAKE_CXX_FLAGS_RELEASE "-Wall -Wextra -Wpedantic -O3")
endif()

include_directories(
  "./src")

add_subdirectory(
  "./src")
