#pragma once

#include "domain/BallState.hpp"
#include "domain/Trajectory.hpp"
#include "domain/PhysicsConfig.hpp"

namespace domain {

// Pure domain service: deterministic physics with fixed timestep
// No I/O, no time functions, no random numbers
class PhysicsEngine {
public:
  explicit PhysicsEngine(const PhysicsConfig& config);
  
  // Update physics state with real-time delta (accumulator pattern)
  void step(double dt_real);
  
  // Start a new shot with launch conditions
  void startShot(const LaunchCondition& launch);
  
  // Check if ball has landed (z <= 0)
  bool hasLanded() const;
  
  // Get current ball state
  const BallState& getCurrentState() const { return current_state_; }
  
  // Get trajectory history
  const Trajectory& getTrajectory() const { return trajectory_; }
  
  // Calculate final shot result
  ShotResult calculateResult() const;
  
  // Reset for next shot
  void reset();
  
  // Check if currently in flight
  bool isInFlight() const { return current_state_.in_flight; }
  
private:
  void integrate(double dt);
  Vec3 computeAcceleration(const BallState& state) const;
  
  PhysicsConfig config_;
  BallState current_state_;
  Trajectory trajectory_;
  double accumulator_ = 0.0;
  Vec3 initial_position_;
};

} // namespace domain
