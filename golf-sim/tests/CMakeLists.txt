cmake_minimum_required(VERSION 3.10)

# Simple test for domain physics
add_executable(test_physics
  test_physics.cpp
)

target_link_libraries(test_physics domain)
target_include_directories(test_physics PRIVATE ${CMAKE_SOURCE_DIR}/include)

add_test(NAME PhysicsTest COMMAND test_physics)

# Screen flow/state transition tests (headless)
add_executable(test_screen_flow
  test_screen_flow.cpp
)
target_link_libraries(test_screen_flow application domain)
target_include_directories(test_screen_flow PRIVATE ${CMAKE_SOURCE_DIR}/include)
add_test(NAME ScreenFlowTest COMMAND test_screen_flow)

# Coordinate converter tests (application layer)
add_executable(test_coordinate_converter
  test_coordinate_converter.cpp
)
target_link_libraries(test_coordinate_converter application domain)
target_include_directories(test_coordinate_converter PRIVATE ${CMAKE_SOURCE_DIR}/include)
add_test(NAME CoordinateConverterTest COMMAND test_coordinate_converter)
