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

# ===== DOMAIN LAYER (Pure C++, no external dependencies) =====
add_library(domain STATIC
  src/domain/Vec3.cpp
  src/domain/Trajectory.cpp
  src/domain/PhysicsEngine.cpp
  src/domain/GameState.cpp
  src/domain/GameStateMachine.cpp
)
target_include_directories(domain PUBLIC include)
# Domain has NO dependencies (pure C++)

# ===== APPLICATION LAYER (depends on domain only) =====
add_library(application STATIC
  src/application/ShotParameterService.cpp
  src/application/UseCases.cpp
  src/application/ScreenFlow.cpp
  src/application/CoordinateConverter.cpp
)
target_include_directories(application PUBLIC include)
target_link_libraries(application PUBLIC domain)

# ===== INFRASTRUCTURE LAYER (depends on application, domain) =====
add_library(infrastructure STATIC
  src/infrastructure/MockSensorProvider.cpp
)
target_include_directories(infrastructure PUBLIC include)
target_link_libraries(infrastructure PUBLIC application domain)

# ===== PRESENTATION LAYER (depends on all, includes raylib) =====
add_library(presentation STATIC
  src/app/App.cpp
  src/render/Renderer.cpp
)
target_include_directories(presentation PUBLIC include)
target_link_libraries(presentation PUBLIC application infrastructure domain raylib)

# ===== MAIN EXECUTABLE (composition root) =====
add_executable(golf-sim
  src/main.cpp
)
target_link_libraries(golf-sim presentation application infrastructure domain raylib)

# Optional: compile flags
if(APPLE)
  target_link_libraries(golf-sim "-framework Cocoa" "-framework IOKit" "-framework CoreVideo")
endif()

# Release optimization
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

# Add tests directory if it exists
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests")
  enable_testing()
  add_subdirectory(tests)
endif()

