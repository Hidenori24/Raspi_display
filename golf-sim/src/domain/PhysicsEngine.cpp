#include "domain/PhysicsEngine.hpp"
#include <cmath>

namespace domain {

PhysicsEngine::PhysicsEngine(const PhysicsConfig& config)
  : config_(config) {
  reset();
}

void PhysicsEngine::startShot(const LaunchCondition& launch) {
  // Convert launch angle and speed to velocity vector
  double angle_rad = launch.launch_angle_deg * M_PI / 180.0;
  
  // Initial velocity (assuming shot is in +y direction)
  current_state_.vel.x = 0.0;
  current_state_.vel.y = launch.launch_speed_mps * std::cos(angle_rad);
  current_state_.vel.z = launch.launch_speed_mps * std::sin(angle_rad);
  
  current_state_.pos = Vec3(0.0, 0.0, 0.0);  // Start at origin
  current_state_.t_sec = 0.0;
  current_state_.spin = launch.initial_spin;
  current_state_.in_flight = true;
  
  initial_position_ = current_state_.pos;
  trajectory_.clear();
  trajectory_.addPoint(current_state_);
  accumulator_ = 0.0;
}

void PhysicsEngine::step(double dt_real) {
  if (!current_state_.in_flight) {
    return;
  }
  
  // Accumulator pattern for fixed timestep
  accumulator_ += dt_real;
  
  while (accumulator_ >= config_.dt_fixed_sec) {
    integrate(config_.dt_fixed_sec);
    accumulator_ -= config_.dt_fixed_sec;
    
    // Check landing condition
    if (current_state_.pos.z <= 0.0 && current_state_.t_sec > 0.01) {
      current_state_.pos.z = 0.0;
      current_state_.vel = Vec3(0.0, 0.0, 0.0);
      current_state_.in_flight = false;
      trajectory_.addPoint(current_state_);
      break;
    }
  }
}

void PhysicsEngine::integrate(double dt) {
  if (!current_state_.in_flight) {
    return;
  }
  
  // Simple Euler integration (can be upgraded to RK4 if needed)
  Vec3 accel = computeAcceleration(current_state_);
  
  current_state_.vel = current_state_.vel + accel * dt;
  current_state_.pos = current_state_.pos + current_state_.vel * dt;
  current_state_.t_sec += dt;
  
  // Store trajectory point (can be downsampled later if needed)
  trajectory_.addPoint(current_state_);
}

Vec3 PhysicsEngine::computeAcceleration(const BallState& state) const {
  Vec3 accel;
  
  // Gravity
  accel.z = -config_.gravity;
  
  // Air resistance (simplified drag model)
  Vec3 v_rel = state.vel - config_.wind_velocity;
  double v_rel_mag = v_rel.length();
  
  if (v_rel_mag > 1e-6) {
    // Drag force: F_d = -k * |v|^2 * v_hat
    // a_d = F_d / m = -k * |v| * v (assuming unit mass)
    Vec3 drag = v_rel.normalized() * (-config_.drag_coefficient * v_rel_mag * v_rel_mag);
    accel = accel + drag;
  }
  
  // TODO: Add Magnus force for spin effects if needed in future
  
  return accel;
}

bool PhysicsEngine::hasLanded() const {
  return !current_state_.in_flight;
}

ShotResult PhysicsEngine::calculateResult() const {
  ShotResult result;
  
  if (trajectory_.empty()) {
    return result;
  }
  
  const BallState& final_state = trajectory_.getLastPoint();
  
  // Carry distance (straight-line distance from start to landing)
  Vec3 displacement = final_state.pos - initial_position_;
  result.carry_m = std::sqrt(displacement.x * displacement.x + 
                             displacement.y * displacement.y);
  
  // Total distance (same as carry for now, can add roll later)
  result.total_m = result.carry_m;
  
  // Lateral distance (x-axis deviation)
  result.lateral_m = final_state.pos.x;
  
  // Flight time
  result.flight_time_s = final_state.t_sec;
  
  // Landing position
  result.landing_position = final_state.pos;
  
  return result;
}

void PhysicsEngine::reset() {
  current_state_ = BallState();
  current_state_.in_flight = false;
  trajectory_.clear();
  accumulator_ = 0.0;
  initial_position_ = Vec3(0.0, 0.0, 0.0);
}

} // namespace domain
