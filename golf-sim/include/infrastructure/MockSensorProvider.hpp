#pragma once

#include "infrastructure/ISensorProvider.hpp"
#include <random>

namespace infrastructure {

// Mock sensor provider with deterministic behavior (seed-based)
// Used for development and testing
class MockSensorProvider : public ISensorProvider {
public:
  enum class Scenario {
    Basic,      // Straight shot
    Slice,      // Right curve
    Hook,       // Left curve
    High,       // High trajectory
    Low         // Low trajectory
  };
  
  explicit MockSensorProvider(Scenario scenario = Scenario::Basic, unsigned int seed = 42);
  
  bool poll(SensorFrame& out) override;
  void reset() override;
  
  // Trigger impact event (for testing)
  void triggerImpact(double speed_mps, double angle_deg);
  
private:
  Scenario scenario_;
  std::mt19937 rng_;
  double current_time_;
  bool impact_triggered_;
  double impact_speed_;
  double impact_angle_;
  int poll_count_;
};

} // namespace infrastructure
