#include "infrastructure/MockSensorProvider.hpp"
#include <cmath>

namespace infrastructure {

MockSensorProvider::MockSensorProvider(Scenario scenario, unsigned int seed)
  : scenario_(scenario)
  , rng_(seed)
  , current_time_(0.0)
  , impact_triggered_(false)
  , impact_speed_(0.0)
  , impact_angle_(0.0)
  , poll_count_(0) {
}

bool MockSensorProvider::poll(SensorFrame& out) {
  // Mock: only provide data when impact is triggered
  if (!impact_triggered_) {
    return false;
  }
  
  // Generate deterministic noise
  std::uniform_real_distribution<float> noise(-0.1f, 0.1f);
  
  // Create frame with impact-like acceleration
  out.t_sec = current_time_;
  out.ax = noise(rng_);
  out.ay = noise(rng_);
  out.az = 20.0f + noise(rng_);  // High z-acceleration for impact
  out.gx = noise(rng_);
  out.gy = noise(rng_);
  out.gz = 10.0f + noise(rng_);  // High rotation
  
  // One-shot: return true once then reset
  impact_triggered_ = false;
  poll_count_++;
  
  return true;
}

void MockSensorProvider::reset() {
  current_time_ = 0.0;
  impact_triggered_ = false;
  poll_count_ = 0;
}

void MockSensorProvider::triggerImpact(double speed_mps, double angle_deg) {
  impact_triggered_ = true;
  impact_speed_ = speed_mps;
  impact_angle_ = angle_deg;
}

} // namespace infrastructure
