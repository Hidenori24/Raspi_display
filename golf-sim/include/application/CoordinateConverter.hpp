#pragma once

#include "domain/Vec3.hpp"
#include "domain/Trajectory.hpp"
#include <vector>

namespace application {

// Coordinate system adapter between Domain (physics) and Presentation (render)
// 
// Domain coordinate system (Physics):
//   - Origin: Tee position at (0, 0, 0)
//   - X: Lateral distance (positive = right)
//   - Y: Downfield distance (positive = towards target)
//   - Z: Height above ground (positive = up)
//
// Presentation coordinate system (Render):
//   - Origin: Screen center
//   - X: Lateral distance (positive = right)
//   - Y: Downfield distance with tee at -17.5 (positive = towards target)
//   - Height: Z coordinate for rendering ball arc
//
// The tee offset constant represents the visual positioning of the tee
// in the render coordinate system to create proper perspective.
class CoordinateConverter {
public:
  // Tee position offset in render coordinates (meters)
  // This positions the tee at the "near" edge of the visible green
  static constexpr float TEE_RENDER_OFFSET_Y = -17.5f;

  // Convert a single domain position to render coordinates
  struct RenderPoint {
    float x;       // Lateral position in render space (unchanged from domain)
    float y;       // Downfield position in render space (domain y + TEE_RENDER_OFFSET_Y)
    float height;  // Height above ground (unchanged from domain z)
  };

  static RenderPoint toRenderCoordinates(const domain::Vec3& domain_pos);
  
  // Convert entire trajectory to render coordinates
  static std::vector<RenderPoint> toRenderTrajectory(const domain::Trajectory& trajectory);
};

} // namespace application
