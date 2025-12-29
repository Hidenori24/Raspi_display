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
  
  // Start in Idle state with Intro screen
  screen_state_ = ScreenState::Intro;
  // state_machine starts in Idle by default
  
  // Default shot parameters
  current_params_.club_index = 0;  // Driver
  current_params_.power = 0.7f;
  current_params_.aim_angle_deg = 0.0f;
}

void App::run() {
  while (!WindowShouldClose()) {
    double dt = GetFrameTime();
    
    handleInput();
    update(dt);
    render();
  }
}

void App::handleInput() {
  // Screen state transitions
  if (screen_state_ == ScreenState::Intro) {
    // Intro screen: SPACE/ENTER to start playing
    if (IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_ENTER)) {
      screen_state_ = ScreenState::Playing;
      state_machine_.transitionToArmed();
      cinematic_view_ = false;  // Start in normal view
      manual_view_override_ = false;
    }
    return;  // No other input during intro
  }
  
  // Playing state: normal game input
  domain::GameState state = state_machine_.getCurrentState();
  
  // View mode toggles
  if (IsKeyPressed(KEY_V)) {
    manual_view_override_ = !manual_view_override_;
  }
  if (IsKeyPressed(KEY_C)) {
    cinematic_view_ = !cinematic_view_;
  }
  
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
      // Ensure automatic view switching on shot
      manual_view_override_ = false;
      cinematic_view_ = false;
      execute_shot_->execute(current_params_);
    }
  }
  else if (state == domain::GameState::Result) {
    // Next hole
    if (IsKeyPressed(KEY_SPACE)) {
      physics_.reset();
      state_machine_.transitionToIdle();
      hole_number_++;
      manual_view_override_ = false;  // Reset view override for next hole
      screen_state_ = ScreenState::Intro;  // Back to intro for next hole
      cinematic_view_ = false;
      
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
  // Handle screen states
  if (screen_state_ == ScreenState::Intro) {
    // Draw intro screen with golfer and course view
    renderer_->drawIntroScreen(hole_number_, 4, 350.0f);  // Par 4, 350 yards
    return;
  }
  
  // Playing state: normal game rendering
  if (cinematic_view_) {
    // Show cinematic golfer silhouette; still honor state machine but no HUD updates here
    renderer_->drawIntroScreen(hole_number_, 4, 350.0f);
    BeginDrawing();
    DrawText("C: exit cinematic | V: overhead/player | SPACE: shoot/next", 20, SCREEN_HEIGHT - 30, 16, LIGHTGRAY);
    EndDrawing();
    return;
  }
  
  BeginDrawing();
 
    domain::GameState state = state_machine_.getCurrentState();

    // Handle state transitions to stabilize view switching
    if (state != last_state_) {
    // On entering flight/result, ensure automatic overhead view
    if (state == domain::GameState::InFlight || state == domain::GameState::Result) {
      manual_view_override_ = false;
      cinematic_view_ = false;
    }
    // On entering Armed, reset overrides to default close-up
    if (state == domain::GameState::Armed) {
      manual_view_override_ = false;
      cinematic_view_ = false;
    }
    last_state_ = state;
  }
  
  // Determine view mode based on game state and manual override
  ViewMode desired_view;
  if (manual_view_override_) {
    // User pressed V - toggle between views
    if (state == domain::GameState::Armed) {
      desired_view = ViewMode::OverheadView;  // Allow overhead in Armed if toggled
    } else {
      desired_view = ViewMode::PlayerView;  // Allow player view in flight if toggled
    }
  } else {
    // Default automatic view switching
    if (state == domain::GameState::Armed) {
      desired_view = ViewMode::PlayerView;  // Close-up for aiming
    } else {
      desired_view = ViewMode::OverheadView;  // Overhead for watching
    }
  }
  
  renderer_->setViewMode(desired_view);
  
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
    
    // Draw aim direction arrow
    BallPosition tee = {0.0f, -17.5f};
    renderer_->drawAimDirection(tee, current_params_.aim_angle_deg, current_params_.power);
    
    // Draw HUD
    DrawText(TextFormat("Hole: %d", hole_number_), 20, 20, 20, WHITE);
    DrawText(TextFormat("Club: %s", club.name), 20, 50, 20, WHITE);
    DrawText(TextFormat("Power: %.0f%%", current_params_.power * 100), 20, 80, 20, WHITE);
    DrawText(TextFormat("Aim: %.1f deg", current_params_.aim_angle_deg), 20, 110, 20, WHITE);
    // Instruction band
    DrawRectangle(10, SCREEN_HEIGHT - 50, SCREEN_WIDTH - 20, 40, {0, 0, 0, 140});
    DrawRectangleLines(10, SCREEN_HEIGHT - 50, SCREEN_WIDTH - 20, 40, {255, 255, 255, 60});
    DrawText("SPACE: shoot | Arrows: club/power | A/D: aim | V: overhead/player | C: cinematic", 20, SCREEN_HEIGHT - 40, 16, {255, 220, 200, 255});
    DrawText("Current view: Player", SCREEN_WIDTH - 220, SCREEN_HEIGHT - 40, 16, {255, 220, 200, 255});
  }
  else if (state == domain::GameState::InFlight || state == domain::GameState::Result) {
    // Flight or result screen (overhead view)
    const domain::Trajectory& traj = physics_.getTrajectory();
    
    // Convert trajectory to render format
    // Physics uses (0,0,0) as tee position, but render uses (0, -17.5) as tee
    green.trajectory.clear();
    for (const auto& point : traj.getPoints()) {
      green.trajectory.push_back({
        static_cast<float>(point.pos.x),
        static_cast<float>(point.pos.y - 17.5),  // Offset to match tee position in render coords
        static_cast<float>(point.pos.z)
      });
    }
    
    if (!traj.empty()) {
      const domain::BallState& last = traj.getLastPoint();
      green.current_ball_pos.x = static_cast<float>(last.pos.x);
      green.current_ball_pos.y = static_cast<float>(last.pos.y - 17.5);  // Offset to match render coords
    }
    
    green.ball_positions = {{0.0f, -17.5f}};  // Tee position
    
    renderer_->drawGreen(green);
    renderer_->drawBalls(green.ball_positions);  // Draw tee and player first (behind trajectory)
    renderer_->drawTrajectory(green);
    if (state == domain::GameState::InFlight) {
      renderer_->drawCurrentBall(green);  // Only draw moving ball during flight
      DrawRectangle(10, SCREEN_HEIGHT - 50, SCREEN_WIDTH - 20, 40, {0, 0, 0, 140});
      DrawRectangleLines(10, SCREEN_HEIGHT - 50, SCREEN_WIDTH - 20, 40, {255, 255, 255, 60});
      DrawText("In-flight | V: overhead/player | C: cinematic", 20, SCREEN_HEIGHT - 40, 16, {255, 220, 200, 255});
      DrawText("Current view: Overhead", SCREEN_WIDTH - 220, SCREEN_HEIGHT - 40, 16, {255, 220, 200, 255});
    }
    
    if (state == domain::GameState::Result) {
      domain::ShotResult result = physics_.calculateResult();
      
      DrawRectangle(SCREEN_WIDTH / 2 - 200, SCREEN_HEIGHT / 2 - 100, 400, 200, {0, 0, 0, 180});
      DrawRectangleLines(SCREEN_WIDTH / 2 - 200, SCREEN_HEIGHT / 2 - 100, 400, 200, {255, 200, 100, 255});
      DrawText("SHOT COMPLETE!", SCREEN_WIDTH / 2 - 140, SCREEN_HEIGHT / 2 - 80, 20, {255, 200, 100, 255});
      DrawText(TextFormat("Carry: %.1f m", result.carry_m), SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 - 40, 18, WHITE);
      DrawText(TextFormat("Total: %.1f m", result.total_m), SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 - 10, 18, WHITE);
      DrawText(TextFormat("Lateral: %.1f m", result.lateral_m), SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 + 20, 18, WHITE);
      DrawText(TextFormat("Time: %.2f s", result.flight_time_s), SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 + 50, 18, WHITE);
      DrawText("SPACE: next hole | V: overhead/player | C: cinematic", SCREEN_WIDTH / 2 - 170, SCREEN_HEIGHT / 2 + 80, 14, {255, 220, 200, 255});
      DrawRectangle(10, SCREEN_HEIGHT - 50, SCREEN_WIDTH - 20, 40, {0, 0, 0, 140});
      DrawRectangleLines(10, SCREEN_HEIGHT - 50, SCREEN_WIDTH - 20, 40, {255, 255, 255, 60});
      DrawText("Result | SPACE: next hole | V: overhead/player | C: cinematic", 20, SCREEN_HEIGHT - 40, 16, {255, 220, 200, 255});
      DrawText("Current view: Overhead", SCREEN_WIDTH - 220, SCREEN_HEIGHT - 40, 16, {255, 220, 200, 255});
    }
  }
  
  EndDrawing();
}

bool App::shouldClose() const {
  return window_open_;
}
