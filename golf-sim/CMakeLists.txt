cmake_minimum_required(VERSION 3.10)
project(golf_sim)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find raylib
find_package(raylib 5.0 QUIET)
if (NOT raylib_FOUND)
  message(STATUS "raylib not found, will be built from source")
  include(FetchContent)
  FetchContent_Declare(raylib
    URL https://github.com/raysan5/raylib/archive/refs/tags/5.0.tar.gz
  )
  FetchContent_GetProperties(raylib)
  if(NOT raylib_POPULATED)
    set(FETCHCONTENT_QUIET NO)
    FetchContent_Populate(raylib)
    set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(BUILD_GAMES OFF CACHE BOOL "" FORCE)
    add_subdirectory(${raylib_SOURCE_DIR} ${raylib_BINARY_DIR})
  endif()
endif()

# Include directories
include_directories(include)

# Main executable
add_executable(golf-sim
  src/main.cpp
  src/app/App.cpp
  src/render/Renderer.cpp
)

target_link_libraries(golf-sim raylib)

# Optional: compile flags
if(APPLE)
  target_link_libraries(golf-sim "-framework Cocoa" "-framework IOKit" "-framework CoreVideo")
endif()

# Release optimization
set(CMAKE_CXX_FLAGS_RELEASE "-O3")
