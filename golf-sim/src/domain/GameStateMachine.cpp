#include "domain/GameStateMachine.hpp"

namespace domain {

GameStateMachine::GameStateMachine()
  : current_state_(GameState::Idle) {
}

void GameStateMachine::transitionToArmed() {
  if (current_state_ == GameState::Idle || current_state_ == GameState::Result) {
    current_state_ = GameState::Armed;
  }
}

void GameStateMachine::transitionToInFlight(PhysicsEngine& physics, const LaunchCondition& launch) {
  if (current_state_ == GameState::Armed) {
    physics.startShot(launch);
    current_state_ = GameState::InFlight;
  }
}

void GameStateMachine::transitionToResult() {
  if (current_state_ == GameState::InFlight) {
    current_state_ = GameState::Result;
  }
}

void GameStateMachine::transitionToIdle() {
  current_state_ = GameState::Idle;
}

bool GameStateMachine::canStartShot() const {
  return current_state_ == GameState::Armed;
}

} // namespace domain
