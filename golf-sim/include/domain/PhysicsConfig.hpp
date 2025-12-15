#pragma once

#include "domain/Vec3.hpp"

namespace domain {

// Pure value object for physics configuration
struct PhysicsConfig {
  double gravity = 9.80665;           // m/s^2
  double drag_coefficient = 0.02;     // Simplified drag
  Vec3 wind_velocity;                 // m/s (x, y, z)
  double dt_fixed_sec = 1.0 / 240.0;  // Fixed timestep for determinism
  
  PhysicsConfig() = default;
};

} // namespace domain
