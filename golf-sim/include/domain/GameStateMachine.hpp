#pragma once

#include "domain/GameState.hpp"
#include "domain/PhysicsEngine.hpp"

namespace domain {

// Pure domain service: game state machine
// No I/O, no time functions, handles state transitions
class GameStateMachine {
public:
  GameStateMachine();
  
  // State queries
  GameState getCurrentState() const { return current_state_; }
  
  // State transitions
  void transitionToArmed();
  void transitionToInFlight(PhysicsEngine& physics, const LaunchCondition& launch);
  void transitionToResult();
  void transitionToIdle();
  
  // Check if can transition
  bool canStartShot() const;
  
private:
  GameState current_state_;
};

} // namespace domain
