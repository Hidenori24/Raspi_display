#pragma once

#include "domain/GameStateMachine.hpp"
#include "domain/PhysicsEngine.hpp"
#include "application/ShotParameterService.hpp"
#include "application/UseCases.hpp"
#include "application/ScreenFlow.hpp"
#include "infrastructure/MockSensorProvider.hpp"
#include <memory>

// Forward declaration to avoid raylib include here
class Renderer;

// Composition Root: assembles all layers
class App {
public:
  App();
  ~App();

  void run();
  bool shouldClose() const;

private:
  void setup();
  void handleInput();
  void update(double dt);
  void render();

  // Configuration
  static const int SCREEN_WIDTH = 1280;
  static const int SCREEN_HEIGHT = 720;
  
  // Domain layer (pure, no dependencies)
  domain::PhysicsConfig physics_config_;
  domain::GameStateMachine state_machine_;
  domain::PhysicsEngine physics_;
  
  // Application layer
  application::ShotParameterService shot_service_;
  application::ShotParameters current_params_;
  std::unique_ptr<application::ExecuteShotUseCase> execute_shot_;
  std::unique_ptr<application::UpdatePhysicsUseCase> update_physics_;
  
  // Infrastructure layer
  std::unique_ptr<infrastructure::MockSensorProvider> sensor_provider_;
  
  // Presentation layer (raylib dependency)
  std::unique_ptr<Renderer> renderer_;
  
  // UI state (presentation concern)
  int hole_number_ = 1;
  bool window_open_ = true;
  application::ScreenFlow screen_flow_{};
};
