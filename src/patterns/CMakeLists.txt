
add_library(patterns
  STATIC
    PatternMatcher.cpp)

file(
  COPY "${CMAKE_CURRENT_SOURCE_DIR}/pat3_v1.txt"
       "${CMAKE_CURRENT_SOURCE_DIR}/pat5.txt"
  DESTINATION "${CMAKE_BINARY_DIR}/bin")
