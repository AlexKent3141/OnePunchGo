
add_library(search
  SHARED
    Current.h
    Node.h
    Search.h
    TreeWorker.h)

set_target_properties(search
  PROPERTIES
    LINKER_LANGUAGE CXX)

find_package(Threads REQUIRED)
    
target_link_libraries(search
  Threads::Threads)
