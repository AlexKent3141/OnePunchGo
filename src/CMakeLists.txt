
add_executable(OPG
  CommsHandler.cpp
  CustomParameters.cpp
  main.cpp)

add_subdirectory("core")
add_subdirectory("patterns")
add_subdirectory("search")
add_subdirectory("tests")

target_link_libraries(OPG
  PRIVATE
    core
    patterns
    search)
