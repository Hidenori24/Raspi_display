#include "app/App.hpp"
#include "render/Renderer.hpp"
#include <raylib.h>
#include <iostream>
#include <cmath>

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
  game_state_.pin_distance = 249.0f;
  game_state_.wind_speed = 5.0f;
  game_state_.wind_direction = 45.0f;
  game_state_.selected_club = 0;
  game_state_.power = 0.5f;
}

void App::run() {
  Renderer renderer;
  renderer.init(SCREEN_WIDTH, SCREEN_HEIGHT);

  // Initialize green data for InFlight phase
  GreenData green;
  green.width = 20.0f;
  green.length = 35.0f;
  
  // Generate sample trajectory
  for (float t = 0; t <= 5.2f; t += 0.1f) {
    float ratio = t / 5.2f;
    float x = -8.0f + 16.0f * ratio;
    float y = -17.5f + 35.0f * ratio;
    float height = 30.0f * ratio * (1.0f - ratio);
    green.trajectory.push_back({x, y, height});
  }
  
  if (!green.trajectory.empty()) {
    green.current_ball_pos.x = green.trajectory.back().x;
    green.current_ball_pos.y = green.trajectory.back().y;
  }
  
  green.carry_distance = 249.0f;
  green.lateral_distance = -2.5f;
  green.ball_positions = {{-5.0f, -17.5f}};
  green.distances_m = {0.0f};

  // Animation state
  float trajectory_progress = 0.0f;
  bool animating = true;
  float setup_timer = 0.0f;
  
  // Phase transition timer
  float phase_timer = 0.0f;

  while (!WindowShouldClose()) {
    update();
    
    // Handle phase transitions
    if (game_state_.phase == GamePhase::Setup) {
      setup_timer += GetFrameTime();
      
      // Press SPACE to start shot
      if (IsKeyPressed(KEY_SPACE)) {
        game_state_.phase = GamePhase::InFlight;
        trajectory_progress = 0.0f;
        animating = true;
      }
    } 
    else if (game_state_.phase == GamePhase::InFlight) {
      // Animate trajectory
      if (animating) {
        trajectory_progress += 0.01f;
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
      size_t trajectory_end = (size_t)(green.trajectory.size() * trajectory_progress);
      if (trajectory_end > green.trajectory.size()) trajectory_end = green.trajectory.size();
      display_green.trajectory.erase(
        display_green.trajectory.begin() + trajectory_end,
        display_green.trajectory.end()
      );
      
      // Render flight
      draw();
      renderer.drawGreen(display_green);
      renderer.drawTrajectory(display_green);
      renderer.drawCurrentBall(display_green);
      renderer.drawBalls(display_green.ball_positions);
      renderer.drawHUD(display_green);
      renderer.present();
    }
    else if (game_state_.phase == GamePhase::Result) {
      phase_timer += GetFrameTime();
      
      // Render result screen (same as InFlight for now)
      draw();
      renderer.drawGreen(green);
      renderer.drawTrajectory(green);
      renderer.drawCurrentBall(green);
      renderer.drawBalls(green.ball_positions);
      renderer.drawHUD(green);
      renderer.present();
      
      // Press SPACE to go back to Setup (next hole)
      if (IsKeyPressed(KEY_SPACE)) {
        game_state_.phase = GamePhase::Setup;
        game_state_.hole_number++;
        game_state_.total_strokes++;
        setup_timer = 0.0f;
      }
    }
    else if (game_state_.phase == GamePhase::Setup) {
      // Draw setup screen
      draw();
      renderer.drawSetupScreen(
        game_state_.pin_distance,
        game_state_.hole_number,
        game_state_.current_par,
        game_state_.club_names[game_state_.selected_club],
        game_state_.wind_speed,
        game_state_.wind_direction
      );
      renderer.present();
    }
    
    // If it's Setup phase, draw here too
    if (game_state_.phase == GamePhase::Setup) {
      draw();
      renderer.drawSetupScreen(
        game_state_.pin_distance,
        game_state_.hole_number,
        game_state_.current_par,
        game_state_.club_names[game_state_.selected_club],
        game_state_.wind_speed,
        game_state_.wind_direction
      );
      renderer.present();
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
