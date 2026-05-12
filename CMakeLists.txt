cmake_minimum_required(VERSION 3.11.0)
project(
  flnifl
  VERSION 0.1.0
  LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
include(FetchContent)

# To speed up the build, we can disable building FLTK's examples and tests
set(FLTK_BUILD_EXAMPLES
    OFF
    CACHE BOOL "Build FLTK examples")
set(FLTK_BUILD_TEST
    OFF
    CACHE BOOL "Build FLTK tests")
set(FLTK_OPTION_SVG
    ON
    CACHE BOOL "Enable SVG support for Fl_Theme")
set(FLTK_OPTION_CAIRO_WINDOW
    OFF
    CACHE BOOL "Use CMake for FLTK build")

FetchContent_Declare(
  fltk
  GIT_REPOSITORY https://github.com/fltk/fltk.git
  GIT_TAG master # Pin to a stable version for reproducibility
)

# Make dependencies available
FetchContent_MakeAvailable(fltk)

# --- Define the MUI header-only library ---
add_library(mui INTERFACE)
target_include_directories(mui INTERFACE
  ${CMAKE_CURRENT_SOURCE_DIR}/mui
  ${CMAKE_CURRENT_SOURCE_DIR}
)

# --- Build all examples ---
file(GLOB EXAMPLE_SOURCES "examples/*.cpp")

foreach(EXAMPLE_SOURCE ${EXAMPLE_SOURCES})
  get_filename_component(EXAMPLE_NAME ${EXAMPLE_SOURCE} NAME_WE)
  add_executable(${EXAMPLE_NAME} ${EXAMPLE_SOURCE})

  # Add the FLTK source and binary directories to the include path.
  target_include_directories(${EXAMPLE_NAME} PRIVATE ${fltk_SOURCE_DIR} ${fltk_BINARY_DIR})

  # Link against mui (for includes) and fltk libraries
  target_link_libraries(${EXAMPLE_NAME} PRIVATE mui fltk fltk_images)
endforeach()
