#include "domain/PhysicsEngine.hpp"
#include "domain/BallState.hpp"
#include <iostream>
#include <cmath>
#include <cassert>

// Simple test framework
#define TEST(name) void test_##name()
#define RUN_TEST(name) do { \
  std::cout << "Running " #name "..." << std::endl; \
  test_##name(); \
  std::cout << "  PASSED" << std::endl; \
} while(0)

TEST(physics_determinism) {
  // Test that same launch conditions produce same result
  domain::PhysicsConfig config;
  config.gravity = 9.80665;
  config.drag_coefficient = 0.02;
  config.wind_velocity = domain::Vec3(0, 0, 0);
  config.dt_fixed_sec = 1.0 / 240.0;
  
  domain::PhysicsEngine physics1(config);
  domain::PhysicsEngine physics2(config);
  
  domain::LaunchCondition launch;
  launch.launch_speed_mps = 68.0;
  launch.launch_angle_deg = 12.0;
  
  physics1.startShot(launch);
  physics2.startShot(launch);
  
  // Simulate for 10 seconds
  for (int i = 0; i < 10 * 60; i++) {  // 60 FPS for 10 seconds
    physics1.step(1.0 / 60.0);
    physics2.step(1.0 / 60.0);
  }
  
  // Results should be identical (deterministic)
  domain::ShotResult result1 = physics1.calculateResult();
  domain::ShotResult result2 = physics2.calculateResult();
  
  assert(std::abs(result1.carry_m - result2.carry_m) < 0.001);
  assert(std::abs(result1.flight_time_s - result2.flight_time_s) < 0.001);
}

TEST(physics_gravity_only) {
  // Test simple ballistic trajectory with no drag or wind
  domain::PhysicsConfig config;
  config.gravity = 9.80665;
  config.drag_coefficient = 0.0;  // No drag
  config.wind_velocity = domain::Vec3(0, 0, 0);
  config.dt_fixed_sec = 1.0 / 240.0;
  
  domain::PhysicsEngine physics(config);
  
  // 45 degree angle should give maximum range (no drag)
  domain::LaunchCondition launch;
  launch.launch_speed_mps = 10.0;
  launch.launch_angle_deg = 45.0;
  
  physics.startShot(launch);
  
  // Simulate until landing
  while (!physics.hasLanded()) {
    physics.step(1.0 / 60.0);
  }
  
  domain::ShotResult result = physics.calculateResult();
  
  // For 45 degrees with no drag: range = v^2 / g
  double expected_range = (10.0 * 10.0) / config.gravity;
  
  // Allow 5% tolerance due to discretization
  assert(std::abs(result.carry_m - expected_range) / expected_range < 0.05);
}

TEST(physics_drag_reduces_distance) {
  // Test that drag reduces distance
  domain::PhysicsConfig config_no_drag;
  config_no_drag.gravity = 9.80665;
  config_no_drag.drag_coefficient = 0.0;
  config_no_drag.wind_velocity = domain::Vec3(0, 0, 0);
  config_no_drag.dt_fixed_sec = 1.0 / 240.0;
  
  domain::PhysicsConfig config_with_drag = config_no_drag;
  config_with_drag.drag_coefficient = 0.02;
  
  domain::PhysicsEngine physics_no_drag(config_no_drag);
  domain::PhysicsEngine physics_with_drag(config_with_drag);
  
  domain::LaunchCondition launch;
  launch.launch_speed_mps = 50.0;
  launch.launch_angle_deg = 15.0;
  
  physics_no_drag.startShot(launch);
  physics_with_drag.startShot(launch);
  
  // Simulate
  while (!physics_no_drag.hasLanded()) {
    physics_no_drag.step(1.0 / 60.0);
  }
  while (!physics_with_drag.hasLanded()) {
    physics_with_drag.step(1.0 / 60.0);
  }
  
  domain::ShotResult result_no_drag = physics_no_drag.calculateResult();
  domain::ShotResult result_with_drag = physics_with_drag.calculateResult();
  
  // Drag should reduce distance
  assert(result_with_drag.carry_m < result_no_drag.carry_m);
}

TEST(physics_trajectory_points) {
  // Test that trajectory is recorded
  domain::PhysicsConfig config;
  config.gravity = 9.80665;
  config.drag_coefficient = 0.02;
  config.dt_fixed_sec = 1.0 / 240.0;
  
  domain::PhysicsEngine physics(config);
  
  domain::LaunchCondition launch;
  launch.launch_speed_mps = 30.0;
  launch.launch_angle_deg = 20.0;
  
  physics.startShot(launch);
  
  // Simulate
  while (!physics.hasLanded()) {
    physics.step(1.0 / 60.0);
  }
  
  const domain::Trajectory& traj = physics.getTrajectory();
  
  // Should have many points
  assert(traj.size() > 100);
  
  // First point should be at origin
  const domain::BallState& first = traj.getPoints()[0];
  assert(std::abs(first.pos.x) < 0.01);
  assert(std::abs(first.pos.y) < 0.01);
  assert(std::abs(first.pos.z) < 0.01);
  
  // Last point should be at ground level
  const domain::BallState& last = traj.getLastPoint();
  assert(std::abs(last.pos.z) < 0.01);
}

int main() {
  std::cout << "=== Domain Physics Tests ===" << std::endl;
  
  RUN_TEST(physics_determinism);
  RUN_TEST(physics_gravity_only);
  RUN_TEST(physics_drag_reduces_distance);
  RUN_TEST(physics_trajectory_points);
  
  std::cout << "\nAll tests passed!" << std::endl;
  return 0;
}
