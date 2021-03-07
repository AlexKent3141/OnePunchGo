
add_library(search
  INTERFACE)

target_include_directories(search
  INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR})

find_package(Threads REQUIRED)
    
target_link_libraries(search
  INTERFACE
    Threads::Threads
    lurien)
