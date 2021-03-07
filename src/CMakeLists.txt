
add_subdirectory("core")
add_subdirectory("patterns")
add_subdirectory("search")
add_subdirectory("tests")
add_subdirectory("opg")

include(FetchContent)

FetchContent_Declare(
  lurien
  GIT_REPOSITORY "git@github.com:AlexKent3141/lurien.git"
  GIT_TAG master)

FetchContent_MakeAvailable(lurien)
