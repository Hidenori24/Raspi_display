#include "app/App.hpp"
#include "render/Renderer.hpp"
#include <raylib.h>

App::App()
  : physics_(physics_config_)
  , shot_service_()
  , sensor_provider_(std::make_unique<infrastructure::MockSensorProvider>(
      infrastructure::MockSensorProvider::Scenario::Basic, 42))
  , renderer_(std::make_unique<Renderer>()) {
  
  // Configure physics
  physics_config_.gravity = 9.80665;
  physics_config_.drag_coefficient = 0.02;
  physics_config_.wind_velocity = domain::Vec3(1.0, 0.0, 0.0);  // 1 m/s wind
  physics_config_.dt_fixed_sec = 1.0 / 240.0;
  
  // Initialize use cases (dependency injection)
  execute_shot_ = std::make_unique<application::ExecuteShotUseCase>(
    state_machine_, physics_, shot_service_);
  update_physics_ = std::make_unique<application::UpdatePhysicsUseCase>(
    state_machine_, physics_);
  
  setup();
}

App::~App() {
  CloseWindow();
}

void App::setup() {
  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Raspberry Pi 5 - Golf Simulator (Refactored)");
  SetTargetFPS(60);
  
  renderer_->init(SCREEN_WIDTH, SCREEN_HEIGHT);
  
  // Start in Armed state (ready to shoot)
  state_machine_.transitionToArmed();
    intro_pending_ = true; // show intro at first armed
  
  // Default shot parameters
  current_params_.club_index = 0;  // Driver
  current_params_.power = 0.7f;
  current_params_.aim_angle_deg = 0.0f;
}

void App::run() {
  
  while (!WindowShouldClose()) {
    // Show intro whenever pending (e.g., before each Armed setup)
    if (intro_pending_) {
      bool intro = true;
      while (intro && !WindowShouldClose()) {
        renderer_->drawIntroScreen(hole_number_, /*par*/4, /*pin_distance*/ static_cast<float>(17.5f));
        if (IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_ENTER)) {
          intro = false;
        }
      }
      intro_pending_ = false;
    }
    double dt = GetFrameTime();
    
    handleInput();
    update(dt);
    render();
  }
}

void App::handleInput() {
  domain::GameState state = state_machine_.getCurrentState();
  
  if (state == domain::GameState::Armed) {
    // Club selection
    if (IsKeyPressed(KEY_LEFT)) {
      current_params_.club_index = 
        (current_params_.club_index - 1 + shot_service_.getClubCount()) % shot_service_.getClubCount();
    }
    if (IsKeyPressed(KEY_RIGHT)) {
      current_params_.club_index = 
        (current_params_.club_index + 1) % shot_service_.getClubCount();
    }
    
    // Power adjustment
    if (IsKeyDown(KEY_UP)) {
      current_params_.power = std::min(1.0f, current_params_.power + 0.01f);
    }
    if (IsKeyDown(KEY_DOWN)) {
      current_params_.power = std::max(0.1f, current_params_.power - 0.01f);
    }
    
    // Aim adjustment
    if (IsKeyDown(KEY_A)) {
      current_params_.aim_angle_deg = std::max(-30.0f, current_params_.aim_angle_deg - 0.5f);
    }
    if (IsKeyDown(KEY_D)) {
      current_params_.aim_angle_deg = std::min(30.0f, current_params_.aim_angle_deg + 0.5f);
    }
    
    // Execute shot
    if (IsKeyPressed(KEY_SPACE)) {
      execute_shot_->execute(current_params_);
    }
  }
  else if (state == domain::GameState::Result) {
    // Next hole
    if (IsKeyPressed(KEY_SPACE)) {
      physics_.reset();
      state_machine_.transitionToArmed();
      hole_number_++;
      intro_pending_ = true; // show intro before next shot
      
      // Reset parameters
      current_params_.power = 0.7f;
      current_params_.aim_angle_deg = 0.0f;
    }
  }
}

void App::update(double dt) {
  // Update physics if in flight
  update_physics_->update(dt);
}

void App::render() {
  BeginDrawing();
  
  domain::GameState state = state_machine_.getCurrentState();
  
  // Prepare green data for rendering
  GreenData green;
  green.width = 20.0f;
  green.length = 35.0f;
  
  if (state == domain::GameState::Armed) {
    // Setup screen
    green.current_ball_pos = {0.0f, -17.5f};
    green.ball_positions = {{0.0f, -17.5f}};
    
    const application::ClubData& club = shot_service_.getClubData(current_params_.club_index);
    
    renderer_->drawGreen(green);
    renderer_->drawBalls(green.ball_positions);
    
    // Draw HUD
    DrawText(TextFormat("Hole: %d", hole_number_), 20, 20, 20, WHITE);
    DrawText(TextFormat("Club: %s", club.name), 20, 50, 20, WHITE);
    DrawText(TextFormat("Power: %.0f%%", current_params_.power * 100), 20, 80, 20, WHITE);
    DrawText(TextFormat("Aim: %.1f deg", current_params_.aim_angle_deg), 20, 110, 20, WHITE);
    DrawText("SPACE to shoot | Arrows: club/power | A/D: aim", 20, SCREEN_HEIGHT - 40, 16, LIGHTGRAY);
  }
  else if (state == domain::GameState::InFlight || state == domain::GameState::Result) {
    // Flight or result screen
    const domain::Trajectory& traj = physics_.getTrajectory();
    
    // Convert trajectory to render format
    green.trajectory.clear();
    for (const auto& point : traj.getPoints()) {
      green.trajectory.push_back({
        static_cast<float>(point.pos.x),
        static_cast<float>(point.pos.y),
        static_cast<float>(point.pos.z)
      });
    }
    
    if (!traj.empty()) {
      const domain::BallState& last = traj.getLastPoint();
      green.current_ball_pos.x = static_cast<float>(last.pos.x);
      green.current_ball_pos.y = static_cast<float>(last.pos.y);
    }
    
    green.ball_positions = {{0.0f, -17.5f}};  // Tee position
    
    renderer_->drawGreen(green);
    renderer_->drawTrajectory(green);
    renderer_->drawCurrentBall(green);
    renderer_->drawBalls(green.ball_positions);
    
    if (state == domain::GameState::Result) {
      domain::ShotResult result = physics_.calculateResult();
      
      DrawRectangle(SCREEN_WIDTH / 2 - 200, SCREEN_HEIGHT / 2 - 100, 400, 200, {0, 0, 0, 180});
      DrawRectangleLines(SCREEN_WIDTH / 2 - 200, SCREEN_HEIGHT / 2 - 100, 400, 200, {255, 200, 100, 255});
      DrawText("SHOT COMPLETE!", SCREEN_WIDTH / 2 - 140, SCREEN_HEIGHT / 2 - 80, 20, {255, 200, 100, 255});
      DrawText(TextFormat("Carry: %.1f m", result.carry_m), SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 - 40, 18, WHITE);
      DrawText(TextFormat("Total: %.1f m", result.total_m), SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 - 10, 18, WHITE);
      DrawText(TextFormat("Lateral: %.1f m", result.lateral_m), SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 + 20, 18, WHITE);
      DrawText(TextFormat("Time: %.2f s", result.flight_time_s), SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 + 50, 18, WHITE);
      DrawText("Press SPACE for next hole", SCREEN_WIDTH / 2 - 140, SCREEN_HEIGHT / 2 + 80, 14, LIGHTGRAY);
    }
  }
  
  EndDrawing();
}

bool App::shouldClose() const {
  return window_open_;
}
