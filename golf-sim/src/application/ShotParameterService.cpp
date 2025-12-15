#include "application/ShotParameterService.hpp"
#include <cmath>

namespace application {

const ClubData ShotParameterService::clubs_[6] = {
  {"Driver",   68.0, 12.0, 250.0},
  {"3-Wood",   55.0, 15.0, 210.0},
  {"5-Iron",   48.0, 18.0, 180.0},
  {"7-Iron",   42.0, 21.0, 155.0},
  {"9-Iron",   38.0, 24.0, 130.0},
  {"Putter",    2.0,  0.0,   3.0}
};

ShotParameterService::ShotParameterService() {
}

domain::LaunchCondition ShotParameterService::createLaunchCondition(const ShotParameters& params) const {
  const ClubData& club = clubs_[params.club_index];
  
  domain::LaunchCondition launch;
  launch.launch_speed_mps = club.base_speed_mps * params.power;
  launch.launch_angle_deg = club.base_angle_deg;
  
  // Aim angle affects lateral component (simplified)
  double angle_rad = club.base_angle_deg * M_PI / 180.0;
  double aim_rad = params.aim_angle_deg * M_PI / 180.0;
  
  // Adjust initial velocity for aim
  launch.initial_velocity.x = launch.launch_speed_mps * std::sin(aim_rad) * std::cos(angle_rad);
  launch.initial_velocity.y = launch.launch_speed_mps * std::cos(aim_rad) * std::cos(angle_rad);
  launch.initial_velocity.z = launch.launch_speed_mps * std::sin(angle_rad);
  
  return launch;
}

const ClubData& ShotParameterService::getClubData(int index) const {
  if (index < 0 || index >= NUM_CLUBS) {
    return clubs_[0];
  }
  return clubs_[index];
}

} // namespace application
