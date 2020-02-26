add_library(angel INTERFACE)
target_include_directories(angel INTERFACE ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(angel INTERFACE tweedledum fmt kitty tweedledee rang percy mockturtle lorina cudd cudd_includes)