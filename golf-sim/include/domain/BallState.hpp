#pragma once

#include "domain/Vec3.hpp"

namespace domain {

// Pure domain entity representing ball state in flight
struct BallState {
  double t_sec = 0.0;
  Vec3 pos;        // Position in meters (x, y, z)
  Vec3 vel;        // Velocity in m/s
  Vec3 spin;       // Spin rate (simplified)
  bool in_flight = false;
  
  BallState() = default;
  BallState(double t, const Vec3& position, const Vec3& velocity)
    : t_sec(t), pos(position), vel(velocity), in_flight(true) {}
};

// Value object for initial launch conditions
struct LaunchCondition {
  Vec3 initial_velocity;  // m/s
  Vec3 initial_spin;      // rpm (simplified)
  double launch_angle_deg = 0.0;
  double launch_speed_mps = 0.0;
  
  LaunchCondition() = default;
  LaunchCondition(double speed, double angle_deg)
    : launch_speed_mps(speed), launch_angle_deg(angle_deg) {}
};

// Value object for shot result
struct ShotResult {
  double carry_m = 0.0;
  double total_m = 0.0;
  double lateral_m = 0.0;  // Positive = right, Negative = left
  double flight_time_s = 0.0;
  Vec3 landing_position;
  
  ShotResult() = default;
};

} // namespace domain
