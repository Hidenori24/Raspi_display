#include "application/CoordinateConverter.hpp"
#include <cassert>
#include <cmath>
#include <iostream>

using namespace application;
using namespace domain;

void testBasicConversion() {
  // Test tee position (0,0,0) -> (0, -17.5, 0)
  Vec3 tee_pos(0.0, 0.0, 0.0);
  auto render_tee = CoordinateConverter::toRenderCoordinates(tee_pos);
  
  assert(std::abs(render_tee.x - 0.0f) < 0.001f);
  assert(std::abs(render_tee.y - (-17.5f)) < 0.001f);  // Expected render coordinate for tee
  assert(std::abs(render_tee.height - 0.0f) < 0.001f);
  
  std::cout << "✓ Tee position converts correctly\n";
}

void testDownfieldConversion() {
  // Test a point 100m downfield at 10m height
  Vec3 downfield_pos(0.0, 100.0, 10.0);
  auto render_pos = CoordinateConverter::toRenderCoordinates(downfield_pos);
  
  assert(std::abs(render_pos.x - 0.0f) < 0.001f);
  assert(std::abs(render_pos.y - (100.0f - 17.5f)) < 0.001f);  // 100m downfield + tee offset
  assert(std::abs(render_pos.height - 10.0f) < 0.001f);
  
  std::cout << "✓ Downfield position converts correctly\n";
}

void testLateralConversion() {
  // Test lateral positions (left/right)
  Vec3 left_pos(-5.0, 50.0, 2.0);
  auto render_left = CoordinateConverter::toRenderCoordinates(left_pos);
  
  assert(std::abs(render_left.x - (-5.0f)) < 0.001f);
  assert(std::abs(render_left.y - (50.0f - 17.5f)) < 0.001f);  // 50m downfield + tee offset
  assert(std::abs(render_left.height - 2.0f) < 0.001f);
  
  Vec3 right_pos(5.0, 50.0, 2.0);
  auto render_right = CoordinateConverter::toRenderCoordinates(right_pos);
  
  assert(std::abs(render_right.x - 5.0f) < 0.001f);
  assert(std::abs(render_right.y - (50.0f - 17.5f)) < 0.001f);  // 50m downfield + tee offset
  assert(std::abs(render_right.height - 2.0f) < 0.001f);
  
  std::cout << "✓ Lateral positions convert correctly\n";
}

void testTrajectoryConversion() {
  // Test converting a trajectory with multiple points
  Trajectory traj;
  traj.addPoint(BallState(0.0, Vec3(0.0, 0.0, 0.0), Vec3(0.0, 0.0, 0.0)));
  traj.addPoint(BallState(1.0, Vec3(1.0, 10.0, 5.0), Vec3(0.0, 0.0, 0.0)));
  traj.addPoint(BallState(2.0, Vec3(2.0, 20.0, 8.0), Vec3(0.0, 0.0, 0.0)));
  
  auto render_points = CoordinateConverter::toRenderTrajectory(traj);
  
  assert(render_points.size() == 3);
  
  // Check first point (tee) - domain (0,0,0) -> render (0,-17.5,0)
  assert(std::abs(render_points[0].x - 0.0f) < 0.001f);
  assert(std::abs(render_points[0].y - (-17.5f)) < 0.001f);
  assert(std::abs(render_points[0].height - 0.0f) < 0.001f);
  
  // Check middle point - domain (1,10,5) -> render (1,-7.5,5)
  assert(std::abs(render_points[1].x - 1.0f) < 0.001f);
  assert(std::abs(render_points[1].y - (10.0f - 17.5f)) < 0.001f);
  assert(std::abs(render_points[1].height - 5.0f) < 0.001f);
  
  // Check last point - domain (2,20,8) -> render (2,2.5,8)
  assert(std::abs(render_points[2].x - 2.0f) < 0.001f);
  assert(std::abs(render_points[2].y - (20.0f - 17.5f)) < 0.001f);
  assert(std::abs(render_points[2].height - 8.0f) < 0.001f);
  
  std::cout << "✓ Trajectory conversion works correctly\n";
}

void testConstantValue() {
  // Verify the constant is documented and has the expected value
  // The constant should match the hardcoded value used throughout the tests
  assert(CoordinateConverter::TEE_RENDER_OFFSET_Y == -17.5f);
  
  // Test that the constant is actually used by the converter (not just defined)
  Vec3 origin(0.0, 0.0, 0.0);
  auto result = CoordinateConverter::toRenderCoordinates(origin);
  assert(result.y == CoordinateConverter::TEE_RENDER_OFFSET_Y);
  
  std::cout << "✓ TEE_RENDER_OFFSET_Y constant has expected value and is used correctly\n";
}

int main() {
  std::cout << "Running CoordinateConverter tests...\n\n";
  
  testBasicConversion();
  testDownfieldConversion();
  testLateralConversion();
  testTrajectoryConversion();
  testConstantValue();
  
  std::cout << "\n✅ All CoordinateConverter tests passed!\n";
  return 0;
}
