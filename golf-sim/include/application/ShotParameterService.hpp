#pragma once

#include "domain/BallState.hpp"

namespace application {

// Application-level shot parameters (user input)
struct ShotParameters {
  int club_index = 0;
  float power = 0.7f;       // 0.0 to 1.0
  float aim_angle_deg = 0.0f;  // -30 to +30
  
  ShotParameters() = default;
};

// Club database (application concern, not domain)
struct ClubData {
  const char* name;
  double base_speed_mps;
  double base_angle_deg;
  double distance_avg_m;
};

// Application service: translate user parameters to domain launch conditions
class ShotParameterService {
public:
  static constexpr int NUM_CLUBS = 6;
  
  ShotParameterService();
  
  // Convert application-level parameters to domain launch condition
  domain::LaunchCondition createLaunchCondition(const ShotParameters& params) const;
  
  // Get club data
  const ClubData& getClubData(int index) const;
  int getClubCount() const { return NUM_CLUBS; }
  
private:
  static const ClubData clubs_[NUM_CLUBS];
};

} // namespace application
