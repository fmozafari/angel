#find_library(TWEEDLEDUM_LIB NAMES tweedledum)
add_library(angel INTERFACE)
target_include_directories(angel INTERFACE ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(angel INTERFACE fmt kitty rang percy mockturtle lorina easy cudd cudd_includes tweedledum_lib eigen)


