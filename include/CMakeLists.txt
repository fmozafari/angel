add_library(starter INTERFACE)
target_include_directories(starter INTERFACE ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(starter INTERFACE tweedledum fmt kitty)
