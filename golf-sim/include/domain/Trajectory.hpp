#pragma once

#include "domain/BallState.hpp"
#include <vector>

namespace domain {

// Pure domain entity for a trajectory (collection of states)
class Trajectory {
public:
  Trajectory() = default;
  
  void addPoint(const BallState& state);
  void clear();
  
  const std::vector<BallState>& getPoints() const { return points_; }
  size_t size() const { return points_.size(); }
  bool empty() const { return points_.empty(); }
  
  const BallState& getLastPoint() const;
  
private:
  std::vector<BallState> points_;
};

} // namespace domain
