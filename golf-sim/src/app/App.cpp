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
  screen_flow_.resetToIntro();
  
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
  if (screen_flow_.screenState() == application::ScreenFlow::ScreenState::Intro) {
    // Intro screen: SPACE/ENTER to start playing
    if (IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_ENTER)) {
      if (screen_flow_.advanceFromIntro()) {
        state_machine_.transitionToArmed();
      }
    }
    return;  // No other input during intro
  }
  
  // Playing state: normal game input
  domain::GameState state = state_machine_.getCurrentState();
  
  // View mode toggles
  if (IsKeyPressed(KEY_V) || IsKeyPressed(KEY_C)) {
    screen_flow_.toggleCinematic();
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
      screen_flow_.onShot();
      execute_shot_->execute(current_params_);
    }
  }
  else if (state == domain::GameState::Result) {
    // Next hole
    if (IsKeyPressed(KEY_SPACE)) {
      physics_.reset();
      state_machine_.transitionToIdle();
      hole_number_++;
      screen_flow_.onNextHole();
      
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
  if (screen_flow_.screenState() == application::ScreenFlow::ScreenState::Intro) {
    // Draw intro screen with golfer and course view
    renderer_->drawIntroScreen(hole_number_, 4, 350.0f);  // Par 4, 350 yards
    return;
  }
  
  BeginDrawing();
 
  domain::GameState state = state_machine_.getCurrentState();

  // Handle state transitions to stabilize view switching
  screen_flow_.onGameStateChange(state);
  bool cinematic = screen_flow_.cinematicEnabled();
  
  // Determine view mode (now fixed to Overhead for stability)
  ViewMode desired_view = screen_flow_.selectView(state);
  
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

    if (cinematic) {
      // Draw silhouette background without text, then overlay HUD for controls
      renderer_->drawIntroSceneLayer(hole_number_, 4, 350.0f, false);
    } else {
      renderer_->drawGreen(green);
      renderer_->drawBalls(green.ball_positions);
      // Draw aim direction arrow
      BallPosition tee = {0.0f, -17.5f};
      renderer_->drawAimDirection(tee, current_params_.aim_angle_deg, current_params_.power);
    }
    
    // Draw HUD (shared)
    DrawText(TextFormat("Hole: %d", hole_number_), 20, 20, 20, WHITE);
    DrawText(TextFormat("Club: %s", club.name), 20, 50, 20, WHITE);
    DrawText(TextFormat("Power: %.0f%%", current_params_.power * 100), 20, 80, 20, WHITE);
    DrawText(TextFormat("Aim: %.1f deg", current_params_.aim_angle_deg), 20, 110, 20, WHITE);
    // Instruction band
    DrawRectangle(10, SCREEN_HEIGHT - 50, SCREEN_WIDTH - 20, 40, {0, 0, 0, 140});
    DrawRectangleLines(10, SCREEN_HEIGHT - 50, SCREEN_WIDTH - 20, 40, {255, 255, 255, 60});
    DrawText("SPACE: shoot | Arrows: club/power | A/D: aim | C/V: toggle silhouette", 20, SCREEN_HEIGHT - 40, 16, {255, 220, 200, 255});
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
      DrawText("In-flight | C/V: toggle silhouette", 20, SCREEN_HEIGHT - 40, 16, {255, 220, 200, 255});
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
      DrawText("SPACE: next hole | C/V: toggle silhouette", SCREEN_WIDTH / 2 - 160, SCREEN_HEIGHT / 2 + 80, 14, {255, 220, 200, 255});
      DrawRectangle(10, SCREEN_HEIGHT - 50, SCREEN_WIDTH - 20, 40, {0, 0, 0, 140});
      DrawRectangleLines(10, SCREEN_HEIGHT - 50, SCREEN_WIDTH - 20, 40, {255, 255, 255, 60});
      DrawText("Result | SPACE: next hole | C/V: toggle silhouette", 20, SCREEN_HEIGHT - 40, 16, {255, 220, 200, 255});
    }
  }
  
  EndDrawing();
}

bool App::shouldClose() const {
  return window_open_;
}
