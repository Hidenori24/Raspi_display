cmake_minimum_required(VERSION 3.10)

# Simple test for domain physics
add_executable(test_physics
  test_physics.cpp
)

target_link_libraries(test_physics domain)
target_include_directories(test_physics PRIVATE ${CMAKE_SOURCE_DIR}/include)

add_test(NAME PhysicsTest COMMAND test_physics)
