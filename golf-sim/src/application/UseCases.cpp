#include "application/UseCases.hpp"

namespace application {

ExecuteShotUseCase::ExecuteShotUseCase(
  domain::GameStateMachine& state_machine,
  domain::PhysicsEngine& physics,
  ShotParameterService& shot_service
) : state_machine_(state_machine)
  , physics_(physics)
  , shot_service_(shot_service) {
}

bool ExecuteShotUseCase::execute(const ShotParameters& params) {
  if (!state_machine_.canStartShot()) {
    return false;
  }
  
  // Convert application parameters to domain launch condition
  domain::LaunchCondition launch = shot_service_.createLaunchCondition(params);
  
  // Transition to InFlight and start physics
  state_machine_.transitionToInFlight(physics_, launch);
  
  return true;
}

UpdatePhysicsUseCase::UpdatePhysicsUseCase(
  domain::GameStateMachine& state_machine,
  domain::PhysicsEngine& physics
) : state_machine_(state_machine)
  , physics_(physics) {
}

void UpdatePhysicsUseCase::update(double dt) {
  if (state_machine_.getCurrentState() != domain::GameState::InFlight) {
    return;
  }
  
  // Update physics
  physics_.step(dt);
  
  // Check for landing
  if (physics_.hasLanded()) {
    state_machine_.transitionToResult();
  }
}

} // namespace application
