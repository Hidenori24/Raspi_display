#include "application/CoordinateConverter.hpp"

namespace application {

CoordinateConverter::RenderPoint CoordinateConverter::toRenderCoordinates(const domain::Vec3& domain_pos) {
  return RenderPoint{
    static_cast<float>(domain_pos.x),                          // X: lateral (unchanged)
    static_cast<float>(domain_pos.y + TEE_RENDER_OFFSET_Y),   // Y: downfield with tee offset
    static_cast<float>(domain_pos.z)                           // Height: vertical (unchanged)
  };
}

std::vector<CoordinateConverter::RenderPoint> CoordinateConverter::toRenderTrajectory(
    const domain::Trajectory& trajectory) {
  std::vector<RenderPoint> render_points;
  render_points.reserve(trajectory.size());
  
  for (const auto& point : trajectory.getPoints()) {
    render_points.push_back(toRenderCoordinates(point.pos));
  }
  
  return render_points;
}

} // namespace application
