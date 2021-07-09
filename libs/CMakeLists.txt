# Distributed under the MIT License (See accompanying file /LICENSE)

if (NOT TARGET bill)
  add_library(bill INTERFACE)
  target_include_directories(bill INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/bill)
endif()

if(ANGEL_USE_EXTERNAL_CLI11)
  if (ANGEL_EXAMPLES AND (NOT TARGET CLI11))
    find_package(CLI11 REQUIRED)
  else()
    find_package(CLI11)
  endif()
else()
  add_library(CLI11 INTERFACE)
  target_include_directories(CLI11 INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/CLI11)
endif()

if (NOT TARGET easy)
  add_library(easy INTERFACE)
  target_include_directories(easy INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/easy)
  target_link_libraries(easy INTERFACE kitty bill)
endif()

if (NOT TARGET ez)
  add_library(ez INTERFACE)
  target_include_directories(ez INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/ez)
endif()

if(ANGEL_USE_EXTERNAL_FMT)
  if (NOT TARGET fmt)
    find_package(fmt REQUIRED)
  endif()
else()
  add_library(fmt INTERFACE)
  target_compile_definitions(fmt INTERFACE ANGEL_FMT_INTERNAL FMT_HEADER_ONLY)
  target_include_directories(fmt INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/fmt)
endif()

if (NOT TARGET glucose)
  add_library(glucose INTERFACE)
  target_include_directories(glucose INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/glucose)
endif()

if (NOT TARGET kitty)
  add_library(kitty INTERFACE)
  target_include_directories(kitty INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/kitty)
endif()

if (NOT TARGET lorina)
  add_library(lorina INTERFACE)
  target_include_directories(lorina INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/lorina)
endif()

if (NOT TARGET mockturtle)
  add_library(mockturtle INTERFACE)
  target_include_directories(mockturtle INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/mockturtle)
  target_link_libraries(mockturtle INTERFACE ez kitty lorina sparsepp percy)
endif()

if(ANGEL_USE_EXTERNAL_JSON)
  find_package(nlohmann_json REQUIRED)
else()
  add_library(nlohmann_json INTERFACE)
  target_include_directories(nlohmann_json INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/nlohmann)
endif()

if (NOT TARGET percy)
  add_library(percy INTERFACE)
  target_include_directories(percy SYSTEM INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/percy)
  target_compile_definitions(percy INTERFACE DISABLE_NAUTY)

  set(THREADS_PREFER_PTHREAD_FLAG ON)
  find_package(Threads REQUIRED)
  target_link_libraries(percy INTERFACE Threads::Threads)

  set(ABC_USE_NAMESPACE "pabc")
  set(STATIC_LIBABC true)
  add_subdirectory(abcsat)
  if (UNIX)
    target_compile_definitions(libabcsat PUBLIC "LIN64" ABC_NAMESPACE=pabc ABC_NO_USE_READLINE)
  elseif(WIN32)
    target_compile_definitions(libabcsat PUBLIC ABC_NAMESPACE=pabc ABC_USE_NO_READLINE NOMINMAX)
    target_compile_definitions(libabcsat PUBLIC WIN32_NO_DLL)
  endif()
  target_link_libraries(percy INTERFACE libabcsat)
endif()

if (NOT TARGET sparsepp)
  add_library(sparsepp INTERFACE)
  target_include_directories(sparsepp INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/sparsepp)
  if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_EQUAL "8.1.0")
    target_compile_options(sparsepp INTERFACE -Wno-class-memaccess)
  endif()
endif()

if(ANGEL_BLAS)
  list(APPEND CMAKE_PREFIX_PATH ${CMAKE_CURRENT_BINARY_DIR}/xtl-0.6.2)
  list(APPEND CMAKE_PREFIX_PATH ${CMAKE_CURRENT_BINARY_DIR}/xtensor-0.20.4)
  add_subdirectory(xtl-0.6.2)
  add_subdirectory(xtensor-0.20.4)
  add_subdirectory(xtensor-blas-0.16.0)
  add_library(xtensor-blas INTERFACE)
  target_include_directories(xtensor-blas INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/xtensor-blas-0.16.0/include)
  target_link_libraries(xtensor-blas INTERFACE xtensor)

  add_definitions(-DHAVE_CBLAS=1)

  if (WIN32)
    find_package(OpenBLAS REQUIRED)
    set(BLAS_LIBRARIES ${CMAKE_INSTALL_PREFIX}${OpenBLAS_LIBRARIES})
  else()
    find_package(BLAS REQUIRED)
    find_package(LAPACK REQUIRED)
  endif()

  message(STATUS "BLAS VENDOR:    " ${BLA_VENDOR})
  message(STATUS "BLAS LIBRARIES: " ${BLAS_LIBRARIES})

  target_link_libraries(xtensor-blas INTERFACE ${BLAS_LIBRARIES} ${LAPACK_LIBRARIES})
endif()

if (NOT TARGET rang)
  add_library(rang INTERFACE)
  target_include_directories(rang INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/rang)
endif()

if (NOT TARGET cudd)
  add_library(cudd_includes INTERFACE)
  target_include_directories(cudd_includes INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/cudd)
  target_include_directories(cudd_includes INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/cudd/epd)
  target_include_directories(cudd_includes INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/cudd/mtr)
  target_include_directories(cudd_includes INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/cudd/st)
  target_include_directories(cudd_includes INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/cudd/cudd)
  add_subdirectory(cudd)
endif()

# Eigen 3.3.8
if (NOT TARGET eigen)
  add_library(eigen INTERFACE)
  target_include_directories(eigen SYSTEM INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/eigen)
endif()

# if (NOT TARGET tweedledum_lib)
#   include(ExternalProject)
#   ExternalProject_Add(tweedledum 
#   SOURCE_DIR  ${CMAKE_CURRENT_SOURCE_DIR}/tweedledum
#   INSTALL_DIR  ${CMAKE_CURRENT_SOURCE_DIR}/tweedledum/install
#   STEP_TARGETS build)
#   add_library(tweedledum_lib INTERFACE)
#   target_include_directories(tweedledum_lib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/tweedledum/include)
#   #find_library(TWEEDLEDUM_LIB NAMES tweedledum)
#   find_library(TWEEDLEDUM_LIB NAMES tweedledum PATHS ${CMAKE_BINARY_DIR}/libs/tweedledum-prefix/src/tweedledum-build/)
#   message("CMAKE_BINARY_DIR=${CMAKE_BINARY_DIR}") 
#   message("TWEELDUM_LIB=${TWEEDLEDUM_LIB}")
#   target_link_libraries(tweedledum_lib INTERFACE ${TWEEDLEDUM_LIB})
# endif()

if (NOT TARGET tweedledum_lib)
  include(ExternalProject)
  set(TWEEDLEDUM_PYBINDS OFF CACHE BOOL "disable Tweedledum's python bindings")
  ExternalProject_Add(tweedledum 
    SOURCE_DIR  ${CMAKE_CURRENT_SOURCE_DIR}/tweedledum
    BINARY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/tweedledum/build
    INSTALL_COMMAND ""
    STEP_TARGETS build)

  add_library(tweedledum_lib INTERFACE)
  target_include_directories(tweedledum_lib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/tweedledum/include)
  target_link_directories(tweedledum_lib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/tweedledum/build)
  message("${CMAKE_CURRENT_SOURCE_DIR}/tweedledum/build")
  add_dependencies(tweedledum_lib tweedledum)
  target_link_libraries(tweedledum_lib INTERFACE tweedledum)
endif()






