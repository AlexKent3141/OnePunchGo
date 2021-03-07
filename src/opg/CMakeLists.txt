
add_executable(OPG
  CommsHandler.cpp
  CustomParameters.cpp
  main.cpp)

target_link_libraries(OPG
  PRIVATE
    core
    patterns
    search)
