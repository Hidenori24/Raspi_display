#include "domain/Trajectory.hpp"
#include <stdexcept>

namespace domain {

void Trajectory::addPoint(const BallState& state) {
  points_.push_back(state);
}

void Trajectory::clear() {
  points_.clear();
}

const BallState& Trajectory::getLastPoint() const {
  if (points_.empty()) {
    throw std::runtime_error("Trajectory is empty");
  }
  return points_.back();
}

} // namespace domain
