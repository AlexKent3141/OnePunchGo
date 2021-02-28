
add_executable(test
  test.cpp)

target_link_libraries(test
  core
  patterns
  search)

file(
  COPY "${CMAKE_CURRENT_SOURCE_DIR}/test_suites"
  DESTINATION "${CMAKE_BINARY_DIR}/bin")
