#include "app/App.hpp"
#include "render/Renderer.hpp"
#include <raylib.h>
#include <iostream>
#include <cmath>
#include <random>
#include <sstream>

// Static initialization
constexpr ClubData GameStateData::clubs[];

// Helper function to generate random hole position
Vector2 generateRandomHolePosition() {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<float> x_dist(-8.0f, 8.0f);
  std::uniform_real_distribution<float> y_dist(15.0f, 17.5f);
  
  return {x_dist(gen), y_dist(gen)};
}

// Generate trajectory based on club selection and power
std::vector<TrajectoryPoint> generateTrajectory(
    const GameStateData& state,
    float aim_angle_rad,
    float hole_x, float hole_y) {
  
  std::vector<TrajectoryPoint> traj;
  
  const ClubData& club = GameStateData::clubs[state.selected_club];
  
  // Distance varies with power and wind
  float distance = club.distance_avg * state.power;
  
  // Apply aim angle (left/right variation)
  float lateral_variation = distance * std::sin(aim_angle_rad) * 0.1f;
  
  // Apply wind effect
  float wind_rad = state.wind_direction * 3.14159f / 180.0f;
  float wind_effect = (state.wind_speed / 10.0f) * distance * 0.05f;
  float wind_x = wind_effect * std::cos(wind_rad);
  float wind_y = wind_effect * std::sin(wind_rad);
  
  // Flight time based on launch angle and distance
  float flight_time = 5.0f * (club.launch_angle / 20.0f) * (state.power + 0.3f);
  float steps = flight_time * 20;  // 20 samples per second
  
  // Generate parabolic trajectory
  for (int i = 0; i <= (int)steps; i++) {
    float t = (float)i / steps;
    
    // Horizontal movement (along y-axis towards hole)
    float y = -17.5f + distance * t + wind_y * t;
    
    // Lateral movement (left-right)
    float x = lateral_variation + wind_x * t;
    
    // Height (parabolic arc)
    float height = 30.0f * (club.launch_angle / 30.0f) * t * (1.0f - t);
    
    traj.push_back({x, y, height});
  }
  
  return traj;
}

App::App() {
  setup();
}

App::~App() {
  CloseWindow();
}

void App::setup() {
  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Raspberry Pi 5 - Golf Simulator");
  SetTargetFPS(60);
  
  // Initialize game state
  game_state_.phase = GamePhase::Setup;
  game_state_.hole_number = 1;
  game_state_.current_par = 4;
  
  // Generate random hole position
  Vector2 hole_pos = generateRandomHolePosition();
  game_state_.hole_x = hole_pos.x;
  game_state_.hole_y = hole_pos.y;
  
  // Calculate distance to hole
  game_state_.pin_distance = std::sqrt(
    game_state_.hole_x * game_state_.hole_x +
    (game_state_.hole_y + 17.5f) * (game_state_.hole_y + 17.5f)
  );
  
  game_state_.wind_speed = 3.0f + (rand() % 50) / 10.0f;  // 3-8 mph
  game_state_.wind_direction = rand() % 360;
  game_state_.selected_club = 0;  // Start with driver
  game_state_.power = 0.7f;
}

void App::run() {
  Renderer renderer;
  renderer.init(SCREEN_WIDTH, SCREEN_HEIGHT);

  // Initialize green data
  GreenData green;
  green.width = 20.0f;
  green.length = 35.0f;
  
  // Animation state
  float trajectory_progress = 0.0f;
  bool animating = false;
  std::vector<TrajectoryPoint> current_trajectory;
  
  float phase_timer = 0.0f;
  float setup_timer = 0.0f;

  while (!WindowShouldClose()) {
    update();
    
    // ===== SETUP PHASE =====
    if (game_state_.phase == GamePhase::Setup) {
      setup_timer += GetFrameTime();
      
      // Club selection (LEFT/RIGHT keys)
      if (IsKeyPressed(KEY_LEFT)) {
        game_state_.selected_club = (game_state_.selected_club - 1 + GameStateData::num_clubs) % GameStateData::num_clubs;
      }
      if (IsKeyPressed(KEY_RIGHT)) {
        game_state_.selected_club = (game_state_.selected_club + 1) % GameStateData::num_clubs;
      }
      
      // Power adjustment (UP/DOWN keys)
      if (IsKeyDown(KEY_UP)) {
        game_state_.power = std::min(1.0f, game_state_.power + 0.01f);
      }
      if (IsKeyDown(KEY_DOWN)) {
        game_state_.power = std::max(0.1f, game_state_.power - 0.01f);
      }
      
      // Aim adjustment (A/D or arrow keys)
      if (IsKeyDown(KEY_A)) {
        game_state_.aim_angle = std::max(-30.0f, game_state_.aim_angle - 0.5f);
      }
      if (IsKeyDown(KEY_D)) {
        game_state_.aim_angle = std::min(30.0f, game_state_.aim_angle + 0.5f);
      }
      
      // Start shot (SPACE)
      if (IsKeyPressed(KEY_SPACE)) {
        game_state_.phase = GamePhase::InFlight;
        
        // Generate trajectory based on current settings
        float aim_rad = game_state_.aim_angle * 3.14159f / 180.0f;
        current_trajectory = generateTrajectory(game_state_, aim_rad, 
                                                game_state_.hole_x, game_state_.hole_y);
        
        trajectory_progress = 0.0f;
        animating = true;
        phase_timer = 0.0f;
      }
      
      // Draw setup screen
      draw();
      renderer.drawSetupScreen(
        game_state_.pin_distance,
        game_state_.hole_number,
        game_state_.current_par,
        GameStateData::clubs[game_state_.selected_club].name,
        game_state_.wind_speed,
        game_state_.wind_direction
      );
      
      // Draw club selection indicator
      DrawText("Club: LEFT/RIGHT | Power: UP/DOWN | Aim: A/D", 20, SCREEN_HEIGHT - 40, 12, {255, 255, 100, 255});
      std::stringstream power_ss;
      power_ss << "Power: " << (int)(game_state_.power * 100) << "%";
      DrawText(power_ss.str().c_str(), 20, SCREEN_HEIGHT - 60, 12, {100, 200, 100, 255});
      
      EndDrawing();
    }
    // ===== IN-FLIGHT PHASE =====
    else if (game_state_.phase == GamePhase::InFlight) {
      if (animating) {
        trajectory_progress += 0.012f;  // Control animation speed
        if (trajectory_progress >= 1.0f) {
          trajectory_progress = 1.0f;
          animating = false;
          phase_timer = 0.0f;
        }
      } else {
        // After animation, wait 2 seconds then go to Result
        phase_timer += GetFrameTime();
        if (phase_timer > 2.0f) {
          game_state_.phase = GamePhase::Result;
        }
      }
      
      // Create animated trajectory
      GreenData display_green = green;
      display_green.trajectory.clear();
      
      size_t trajectory_end = (size_t)(current_trajectory.size() * trajectory_progress);
      if (trajectory_end > current_trajectory.size()) trajectory_end = current_trajectory.size();
      
      for (size_t i = 0; i < trajectory_end; i++) {
        display_green.trajectory.push_back(current_trajectory[i]);
      }
      
      // Set current ball position (last trajectory point)
      if (!display_green.trajectory.empty()) {
        display_green.current_ball_pos.x = display_green.trajectory.back().x;
        display_green.current_ball_pos.y = display_green.trajectory.back().y;
      }
      
      // Calculate carry distance
      if (!current_trajectory.empty()) {
        const TrajectoryPoint& last = current_trajectory.back();
        display_green.carry_distance = std::sqrt(
          last.x * last.x + (last.y + 17.5f) * (last.y + 17.5f)
        );
        display_green.lateral_distance = last.x;
      }
      
      // Show tee position
      display_green.ball_positions = {{0.0f, -17.5f}};
      display_green.distances_m = {0.0f};
      
      // Render flight
      draw();
      renderer.drawGreen(display_green);
      renderer.drawTrajectory(display_green);
      renderer.drawCurrentBall(display_green);
      renderer.drawBalls(display_green.ball_positions);
      renderer.drawHUD(display_green);
      EndDrawing();
    }
    // ===== RESULT PHASE =====
    else if (game_state_.phase == GamePhase::Result) {
      phase_timer += GetFrameTime();
      
      // Prepare result green with final ball position
      GreenData result_green = green;
      result_green.trajectory = current_trajectory;
      
      if (!current_trajectory.empty()) {
        result_green.current_ball_pos.x = current_trajectory.back().x;
        result_green.current_ball_pos.y = current_trajectory.back().y;
        
        result_green.carry_distance = std::sqrt(
          current_trajectory.back().x * current_trajectory.back().x +
          (current_trajectory.back().y + 17.5f) * (current_trajectory.back().y + 17.5f)
        );
        result_green.lateral_distance = current_trajectory.back().x;
      }
      
      result_green.ball_positions = {{0.0f, -17.5f}};
      result_green.distances_m = {0.0f};
      
      // Render result
      draw();
      renderer.drawGreen(result_green);
      renderer.drawTrajectory(result_green);
      renderer.drawCurrentBall(result_green);
      renderer.drawBalls(result_green.ball_positions);
      renderer.drawHUD(result_green);
      
      // Result overlay
      DrawRectangle(SCREEN_WIDTH / 2 - 200, SCREEN_HEIGHT / 2 - 80, 400, 160, {0, 0, 0, 180});
      DrawRectangleLines(SCREEN_WIDTH / 2 - 200, SCREEN_HEIGHT / 2 - 80, 400, 160, {255, 200, 100, 255});
      DrawText("SHOT COMPLETE!", SCREEN_WIDTH / 2 - 140, SCREEN_HEIGHT / 2 - 60, 20, {255, 200, 100, 255});
      DrawText("Press SPACE for next hole", SCREEN_WIDTH / 2 - 160, SCREEN_HEIGHT / 2 + 10, 14, {200, 200, 200, 255});
      
      BeginDrawing();
      EndDrawing();
      
      // Press SPACE to go to next hole
      if (IsKeyPressed(KEY_SPACE)) {
        game_state_.phase = GamePhase::Setup;
        game_state_.hole_number++;
        game_state_.total_strokes++;
        
        // Generate new hole
        Vector2 hole_pos = generateRandomHolePosition();
        game_state_.hole_x = hole_pos.x;
        game_state_.hole_y = hole_pos.y;
        
        game_state_.pin_distance = std::sqrt(
          game_state_.hole_x * game_state_.hole_x +
          (game_state_.hole_y + 17.5f) * (game_state_.hole_y + 17.5f)
        );
        
        game_state_.wind_speed = 3.0f + (rand() % 50) / 10.0f;
        game_state_.wind_direction = rand() % 360;
        
        setup_timer = 0.0f;
      }
    }
  }
}

bool App::shouldClose() const {
  return window_open_;
}

void App::update() {
  // TODO: Update game state based on input
}

void App::draw() {
  // Renderer handles drawing
}
