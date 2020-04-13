include_directories(catch2) # v2.2.1

file(GLOB_RECURSE FILENAMES *.cpp)

add_executable(run_tests ${FILENAMES})
if (ENABLE_COVERAGE)
  target_link_libraries(run_tests PUBLIC angel gcov)
else()
  target_link_libraries(run_tests PUBLIC angel)
endif()

