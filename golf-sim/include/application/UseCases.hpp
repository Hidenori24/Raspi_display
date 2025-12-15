#pragma once

#include "domain/GameStateMachine.hpp"
#include "domain/PhysicsEngine.hpp"
#include "application/ShotParameterService.hpp"

namespace application {

// Use case: Execute a shot
class ExecuteShotUseCase {
public:
  ExecuteShotUseCase(
    domain::GameStateMachine& state_machine,
    domain::PhysicsEngine& physics,
    ShotParameterService& shot_service
  );
  
  // Execute shot with given parameters
  bool execute(const ShotParameters& params);
  
private:
  domain::GameStateMachine& state_machine_;
  domain::PhysicsEngine& physics_;
  ShotParameterService& shot_service_;
};

// Use case: Update physics during flight
class UpdatePhysicsUseCase {
public:
  UpdatePhysicsUseCase(
    domain::GameStateMachine& state_machine,
    domain::PhysicsEngine& physics
  );
  
  // Update physics, check for landing
  void update(double dt);
  
private:
  domain::GameStateMachine& state_machine_;
  domain::PhysicsEngine& physics_;
};

} // namespace application
