#include "render/Renderer.hpp"
#include <raylib.h>
#include <cmath>
#include <sstream>
#include <iomanip>

Renderer::Renderer() = default;

Renderer::~Renderer() = default;

void Renderer::init(int width, int height) {
  screen_width_ = width;
  screen_height_ = height;
  scale_factor_ = 30.0f;
  updatePerspectiveParams();
}

void Renderer::updatePerspectiveParams() {
  persp_.vanish_x = screen_width_ / 2.0f;
  persp_.vanish_y = screen_height_ / 2.0f;
  persp_.near_y = screen_height_ * 0.75f;
  persp_.far_y = screen_height_ * 0.15f;
  persp_.near_width = 20.0f * scale_factor_;  // green.width
  persp_.perspective_scale = 0.5f;
  persp_.far_width = persp_.near_width * persp_.perspective_scale;
}

Vector2 Renderer::mapGreenCoordToScreen(float green_x, float green_y) {
  // green_x: -10 to +10 (width)
  // green_y: -17.5 to +17.5 (length, towards hole)
  
  float x_ratio = (green_x + 10.0f) / 20.0f;  // [0, 1]
  float y_ratio = (green_y + 17.5f) / 35.0f;  // [0, 1]
  
  float depth_scale = 1.0f - y_ratio * (1.0f - persp_.perspective_scale);
  float near_left = persp_.vanish_x - persp_.near_width / 2;
  float far_left = persp_.vanish_x - persp_.far_width / 2;
  
  float screen_x = near_left + (far_left - near_left) * y_ratio + 
                   (persp_.near_width * depth_scale - persp_.near_width * 0.5f) * x_ratio;
  float screen_y = persp_.near_y + (persp_.far_y - persp_.near_y) * y_ratio;
  
  return {screen_x, screen_y};
}

void Renderer::drawGreen(const GreenData& green) {
  // Clear background (sky blue)
  ClearBackground({135, 206, 235, 255});

  // Sky to horizon gradient
  for (int y = 0; y < screen_height_ / 2; y++) {
    float ratio = (float)y / (screen_height_ / 2.0f);
    Color sky = {
      (unsigned char)(135 + (100 - 135) * ratio),
      (unsigned char)(206 + (150 - 206) * ratio),
      (unsigned char)(235 + (100 - 235) * ratio),
      255
    };
    DrawLine(0, y, screen_width_, y, sky);
  }

  // Perspective parameters
  float near_left = persp_.vanish_x - persp_.near_width / 2;
  float near_right = persp_.vanish_x + persp_.near_width / 2;
  float far_left = persp_.vanish_x - persp_.far_width / 2;
  float far_right = persp_.vanish_x + persp_.far_width / 2;

  // Draw green trapezoid
  Vector2 green_corners[4] = {
    {near_left, persp_.near_y},   // near-left
    {near_right, persp_.near_y},  // near-right
    {far_right, persp_.far_y},    // far-right
    {far_left, persp_.far_y}      // far-left
  };

  DrawTriangle(green_corners[0], green_corners[1], green_corners[2], {80, 200, 80, 255});
  DrawTriangle(green_corners[0], green_corners[2], green_corners[3], {80, 200, 80, 255});

  // Draw border
  for (int i = 0; i < 4; i++) {
    Vector2 p1 = green_corners[i];
    Vector2 p2 = green_corners[(i + 1) % 4];
    DrawLineEx(p1, p2, 3, {30, 120, 30, 255});
  }

  // Draw perspective grid lines (depth)
  Color grid_color = {120, 180, 120, 100};
  for (float i = 0; i <= 1.0f; i += 0.1f) {
    float left_x = near_left + (far_left - near_left) * i;
    float right_x = near_right + (far_right - near_right) * i;
    float y = persp_.near_y + (persp_.far_y - persp_.near_y) * i;
    DrawLineEx({left_x, y}, {right_x, y}, 1, grid_color);
  }

  // Draw width lines
  for (float i = 0.1f; i < 1.0f; i += 0.1f) {
    float left_near = near_left + (near_right - near_left) * i;
    float left_far = far_left + (far_right - far_left) * i;
    DrawLineEx({left_near, persp_.near_y}, {left_far, persp_.far_y}, 1, grid_color);
  }

  // Draw distance labels
  for (float i = 0; i <= 17.5f; i += 5) {
    float ratio = i / 17.5f;
    float label_x = persp_.vanish_x + (persp_.far_width / 2 - persp_.near_width / 2) * ratio * 0.5f;
    float label_y = persp_.near_y + (persp_.far_y - persp_.near_y) * ratio;
    
    std::stringstream ss;
    ss << (int)i << "y";
    DrawText(ss.str().c_str(), (int)(label_x + 10), (int)label_y, 12, {40, 80, 40, 200});
  }

  // Draw hole marker
  Vector2 hole_pos = mapGreenCoordToScreen(0, 17.5f);
  DrawCircle((int)hole_pos.x, (int)hole_pos.y, 6, {255, 100, 100, 255});
  DrawText("HOLE", (int)(hole_pos.x - 20), (int)(hole_pos.y - 20), 12, {255, 100, 100, 255});
}

void Renderer::drawTrajectory(const GreenData& green) {
  if (green.trajectory.size() < 2) return;

  // Draw trajectory as a series of circles/dots with arc effect
  for (size_t i = 0; i < green.trajectory.size() - 1; i++) {
    Vector2 p1 = mapGreenCoordToScreen(green.trajectory[i].x, green.trajectory[i].y);
    Vector2 p2 = mapGreenCoordToScreen(green.trajectory[i + 1].x, green.trajectory[i + 1].y);
    
    // Color gradient: white to red
    float ratio = (float)i / green.trajectory.size();
    unsigned char r = (unsigned char)(255);
    unsigned char g = (unsigned char)(200 * (1.0f - ratio));
    unsigned char b = (unsigned char)(200 * (1.0f - ratio));
    Color line_color = {r, g, b, 200};
    
    DrawLineEx(p1, p2, 3, line_color);
  }

  // Draw trajectory end point (landing position)
  if (!green.trajectory.empty()) {
    Vector2 end_pos = mapGreenCoordToScreen(
      green.trajectory.back().x,
      green.trajectory.back().y
    );
    DrawCircle((int)end_pos.x, (int)end_pos.y, 7, {255, 50, 50, 255});
  }
}

void Renderer::drawCurrentBall(const GreenData& green) {
  Vector2 ball_pos = mapGreenCoordToScreen(green.current_ball_pos.x, green.current_ball_pos.y);
  
  // Ball size depends on distance
  float distance_from_hole = std::sqrt(
    green.current_ball_pos.x * green.current_ball_pos.x +
    (green.current_ball_pos.y + 17.5f) * (green.current_ball_pos.y + 17.5f)
  );
  float size_scale = std::max(0.3f, 1.0f - distance_from_hole / 35.0f);  // Prevent negative
  float ball_radius = std::max(4.0f, 15.0f * size_scale);  // Minimum 4 pixels
  
  // Shadow
  DrawCircle((int)ball_pos.x, (int)(ball_pos.y + 3), (int)std::max(2.0f, ball_radius * 0.7f), {50, 50, 50, 100});
  
  // Ball (white with red marking)
  DrawCircle((int)ball_pos.x, (int)ball_pos.y, (int)ball_radius, {255, 255, 255, 255});
  if (ball_radius > 5.0f) {
    DrawCircle((int)(ball_pos.x - 3), (int)(ball_pos.y - 2), (int)(ball_radius / 3), {255, 100, 100, 255});
  }
  
  // Glow effect
  DrawCircleLines((int)ball_pos.x, (int)ball_pos.y, (int)(ball_radius + 2), {255, 255, 100, 100});
}

void Renderer::drawBalls(const std::vector<BallPosition>& positions) {
  // Draw previous shots (tee position - larger)
  for (size_t i = 0; i < positions.size(); ++i) {
    Vector2 screen_pos = mapGreenCoordToScreen(positions[i].x, positions[i].y);
    
    // Draw tee marker (yellow cross)
    DrawLineEx({screen_pos.x - 8, screen_pos.y}, {screen_pos.x + 8, screen_pos.y}, 2, {255, 255, 150, 200});
    DrawLineEx({screen_pos.x, screen_pos.y - 8}, {screen_pos.x, screen_pos.y + 8}, 2, {255, 255, 150, 200});
    DrawCircleLines((int)screen_pos.x, (int)screen_pos.y, 6, {255, 255, 100, 200});
  }
}

void Renderer::drawHUD(const GreenData& green) {
  // Draw semi-transparent background for HUD
  DrawRectangle(10, 10, 420, 200, {0, 0, 0, 120});
  DrawRectangleLines(10, 10, 420, 200, {255, 200, 100, 200});

  // Title
  DrawText("CURRENT SHOT", 20, 20, 18, {255, 200, 100, 255});

  // Distance info (like the game image)
  std::stringstream carry_ss;
  carry_ss << std::fixed << std::setprecision(1) << green.carry_distance;
  std::stringstream lateral_ss;
  lateral_ss << std::fixed << std::setprecision(1) << green.lateral_distance;

  // Carry distance (big)
  DrawText(carry_ss.str().c_str(), 30, 50, 28, {255, 255, 100, 255});
  DrawText("y", 120, 55, 20, {255, 255, 100, 255});
  DrawText("Carry", 30, 82, 14, {200, 200, 200, 255});

  // Lateral deviation
  DrawText("Lateral:", 30, 110, 14, {200, 200, 200, 255});
  DrawText(lateral_ss.str().c_str(), 140, 110, 18, {100, 200, 255, 255});
  DrawText("y", 200, 113, 14, {100, 200, 255, 255});

  // Additional stats
  DrawText("Flight Time: 5.2s", 30, 140, 12, {180, 180, 180, 255});
  DrawText("Spin: 2500 rpm", 30, 160, 12, {180, 180, 180, 255});

  // Right side: Club info
  DrawText("CLUB", 250, 20, 14, {255, 200, 100, 255});
  DrawText("Driver", 250, 40, 16, {200, 200, 200, 255});
  DrawText("Speed: 68 mph", 250, 65, 12, {180, 180, 180, 255});
  DrawText("Angle: 12°", 250, 85, 12, {180, 180, 180, 255});
  DrawText("Distance: 210y", 250, 105, 12, {180, 180, 180, 255});

  // FPS display
  std::stringstream fps_ss;
  fps_ss << "FPS: " << GetFPS();
  DrawText(fps_ss.str().c_str(), screen_width_ - 150, 10, 14, {255, 255, 0, 255});
}

void Renderer::drawSetupScreen(float pin_distance, int hole_number, int par, 
                               const char* club_name, float wind_speed, float wind_angle) {
  BeginDrawing();
  // Clear with light green grass background
  ClearBackground({100, 180, 80, 255});
  
  // Draw sky gradient
  for (int y = 0; y < screen_height_ / 2; y++) {
    float ratio = (float)y / (screen_height_ / 2.0f);
    Color sky = {
      (unsigned char)(135 + (100 - 135) * ratio),
      (unsigned char)(206 + (150 - 206) * ratio),
      (unsigned char)(235 + (100 - 235) * ratio),
      255
    };
    DrawLine(0, y, screen_width_, y, sky);
  }
  
  // Draw simple course ground (rough/fairway)
  DrawRectangle(0, screen_height_ / 2, screen_width_, screen_height_ / 2, {80, 160, 60, 255});
  
  // ===== TOP LEFT: Hole Info =====
  DrawRectangle(20, 20, 180, 120, {0, 0, 0, 140});
  DrawRectangleLines(20, 20, 180, 120, {255, 200, 100, 200});
  
  std::stringstream hole_ss;
  hole_ss << "Hole " << hole_number;
  DrawText(hole_ss.str().c_str(), 35, 35, 20, {255, 200, 100, 255});
  
  std::stringstream par_ss;
  par_ss << "PAR " << par;
  DrawText(par_ss.str().c_str(), 35, 65, 18, {200, 200, 200, 255});
  
  std::stringstream distance_ss;
  distance_ss << (int)pin_distance << "y";
  DrawText(distance_ss.str().c_str(), 35, 95, 24, {255, 255, 100, 255});
  
  // ===== TOP RIGHT: Wind Info =====
  DrawRectangle(screen_width_ - 200, 20, 180, 120, {0, 0, 0, 140});
  DrawRectangleLines(screen_width_ - 200, 20, 180, 120, {100, 150, 255, 200});
  
  DrawText("WIND", screen_width_ - 185, 35, 16, {100, 150, 255, 255});
  
  std::stringstream wind_ss;
  wind_ss << wind_speed << " mph";
  DrawText(wind_ss.str().c_str(), screen_width_ - 185, 60, 18, {200, 200, 200, 255});
  
  // Draw wind direction arrow
  float center_x = screen_width_ - 100;
  float center_y = 100;
  float arrow_length = 30.0f;
  float angle_rad = wind_angle * 3.14159f / 180.0f;
  float end_x = center_x + arrow_length * std::cos(angle_rad);
  float end_y = center_y - arrow_length * std::sin(angle_rad);
  
  DrawLineEx({center_x, center_y}, {end_x, end_y}, 3, {100, 150, 255, 255});
  DrawTriangle(
    {end_x, end_y},
    {end_x - 8 * std::cos(angle_rad + 0.5f), end_y + 8 * std::sin(angle_rad + 0.5f)},
    {end_x - 8 * std::cos(angle_rad - 0.5f), end_y + 8 * std::sin(angle_rad - 0.5f)},
    {100, 150, 255, 255}
  );
  
  // ===== CENTER: Club Selection =====
  DrawRectangle(screen_width_ / 2 - 250, screen_height_ / 2 - 80, 500, 160, {0, 0, 0, 140});
  DrawRectangleLines(screen_width_ / 2 - 250, screen_height_ / 2 - 80, 500, 160, {200, 200, 100, 200});
  
  DrawText("SELECT CLUB", screen_width_ / 2 - 220, screen_height_ / 2 - 65, 18, {200, 200, 100, 255});
  
  // Draw selected club in large text
  DrawText(club_name, screen_width_ / 2 - 100, screen_height_ / 2 - 20, 32, {255, 255, 100, 255});
  
  DrawText("Ready to swing!", screen_width_ / 2 - 150, screen_height_ / 2 + 30, 16, {200, 200, 200, 255});
  DrawText("Press SPACE to begin", screen_width_ / 2 - 160, screen_height_ / 2 + 55, 14, {180, 180, 180, 255});
  
  // ===== BOTTOM: Power and Aim Indicators =====
  DrawRectangle(20, screen_height_ - 140, screen_width_ - 40, 120, {0, 0, 0, 140});
  DrawRectangleLines(20, screen_height_ - 140, screen_width_ - 40, 120, {100, 200, 100, 200});
  
  DrawText("POWER:", 40, screen_height_ - 120, 16, {100, 200, 100, 255});
  // Power bar (placeholder)
  DrawRectangle(150, screen_height_ - 115, 200, 20, {100, 100, 100, 200});
  DrawRectangleLines(150, screen_height_ - 115, 200, 20, {100, 200, 100, 200});
  DrawRectangle(150, screen_height_ - 115, 100, 20, {255, 200, 50, 255});  // 50% power
  
  DrawText("AIM:", 40, screen_height_ - 80, 16, {100, 200, 100, 255});
  // Simple aim display (crosshair-like)
  float aim_center_x = 160;
  float aim_center_y = screen_height_ - 60;
  DrawLineEx({aim_center_x - 15, aim_center_y}, {aim_center_x + 15, aim_center_y}, 2, {100, 200, 100, 255});
  DrawLineEx({aim_center_x, aim_center_y - 15}, {aim_center_x, aim_center_y + 15}, 2, {100, 200, 100, 255});
  DrawCircleLines((int)aim_center_x, (int)aim_center_y, 20, {100, 200, 100, 200});
  
  // Stats display on the right
  DrawText("ROUND STATS:", screen_width_ - 300, screen_height_ - 120, 14, {150, 150, 150, 255});
  DrawText("Strokes: 0", screen_width_ - 300, screen_height_ - 95, 12, {180, 180, 180, 255});
  DrawText("Distance: 0y", screen_width_ - 300, screen_height_ - 72, 12, {180, 180, 180, 255});
  
  // FPS
  std::stringstream fps_ss;
  fps_ss << "FPS: " << GetFPS();
  DrawText(fps_ss.str().c_str(), screen_width_ - 150, 10, 14, {255, 255, 0, 255});
  
  EndDrawing();
}

void Renderer::drawSetupScreenWithGreen(float pin_distance, int hole_number, int par, 
                                const char* club_name, float wind_speed, float wind_angle,
                                const GreenData& green) {
  BeginDrawing();
  // Draw the green and tee marker
  drawGreen(green);
  drawBalls(green.ball_positions);
  drawCurrentBall(green);

  // Overlay UI identical to drawSetupScreen (without re-clearing background)
  DrawRectangle(20, 20, 180, 120, {0, 0, 0, 140});
  DrawRectangleLines(20, 20, 180, 120, {255, 200, 100, 200});

  std::stringstream hole_ss;
  hole_ss << "Hole " << hole_number;
  DrawText(hole_ss.str().c_str(), 35, 35, 20, {255, 200, 100, 255});

  std::stringstream par_ss;
  par_ss << "PAR " << par;
  DrawText(par_ss.str().c_str(), 35, 65, 18, {200, 200, 200, 255});

  std::stringstream distance_ss;
  distance_ss << (int)pin_distance << "y";
  DrawText(distance_ss.str().c_str(), 35, 95, 24, {255, 255, 100, 255});

  // Wind info
  DrawRectangle(screen_width_ - 200, 20, 180, 120, {0, 0, 0, 140});
  DrawRectangleLines(screen_width_ - 200, 20, 180, 120, {100, 150, 255, 200});

  DrawText("WIND", screen_width_ - 185, 35, 16, {100, 150, 255, 255});

  std::stringstream wind_ss;
  wind_ss << wind_speed << " mph";
  DrawText(wind_ss.str().c_str(), screen_width_ - 185, 60, 18, {200, 200, 200, 255});

  float center_x = screen_width_ - 100;
  float center_y = 100;
  float arrow_length = 30.0f;
  float angle_rad = wind_angle * 3.14159f / 180.0f;
  float end_x = center_x + arrow_length * std::cos(angle_rad);
  float end_y = center_y - arrow_length * std::sin(angle_rad);

  DrawLineEx({center_x, center_y}, {end_x, end_y}, 3, {100, 150, 255, 255});
  DrawTriangle(
    {end_x, end_y},
    {end_x - 8 * std::cos(angle_rad + 0.5f), end_y + 8 * std::sin(angle_rad + 0.5f)},
    {end_x - 8 * std::cos(angle_rad - 0.5f), end_y + 8 * std::sin(angle_rad - 0.5f)},
    {100, 150, 255, 255}
  );

  // Center box
  DrawRectangle(screen_width_ / 2 - 250, screen_height_ / 2 - 80, 500, 160, {0, 0, 0, 140});
  DrawRectangleLines(screen_width_ / 2 - 250, screen_height_ / 2 - 80, 500, 160, {200, 200, 100, 200});

  DrawText("SELECT CLUB", screen_width_ / 2 - 220, screen_height_ / 2 - 65, 18, {200, 200, 100, 255});
  DrawText(club_name, screen_width_ / 2 - 100, screen_height_ / 2 - 20, 32, {255, 255, 100, 255});
  DrawText("Ready to swing!", screen_width_ / 2 - 150, screen_height_ / 2 + 30, 16, {200, 200, 200, 255});
  DrawText("Press SPACE to begin", screen_width_ / 2 - 160, screen_height_ / 2 + 55, 14, {180, 180, 180, 255});

  // Bottom bar
  DrawRectangle(20, screen_height_ - 140, screen_width_ - 40, 120, {0, 0, 0, 140});
  DrawRectangleLines(20, screen_height_ - 140, screen_width_ - 40, 120, {100, 200, 100, 200});

  DrawText("POWER:", 40, screen_height_ - 120, 16, {100, 200, 100, 255});
  DrawRectangle(150, screen_height_ - 115, 200, 20, {100, 100, 100, 200});
  DrawRectangleLines(150, screen_height_ - 115, 200, 20, {100, 200, 100, 200});
  DrawRectangle(150, screen_height_ - 115, 100, 20, {255, 200, 50, 255});

  DrawText("AIM:", 40, screen_height_ - 80, 16, {100, 200, 100, 255});
  float aim_center_x = 160;
  float aim_center_y = screen_height_ - 60;
  DrawLineEx({aim_center_x - 15, aim_center_y}, {aim_center_x + 15, aim_center_y}, 2, {100, 200, 100, 255});
  DrawLineEx({aim_center_x, aim_center_y - 15}, {aim_center_x, aim_center_y + 15}, 2, {100, 200, 100, 255});
  DrawCircleLines((int)aim_center_x, (int)aim_center_y, 20, {100, 200, 100, 200});

  DrawText("ROUND STATS:", screen_width_ - 300, screen_height_ - 120, 14, {150, 150, 150, 255});
  DrawText("Strokes: 0", screen_width_ - 300, screen_height_ - 95, 12, {180, 180, 180, 255});
  DrawText("Distance: 0y", screen_width_ - 300, screen_height_ - 72, 12, {180, 180, 180, 255});

  std::stringstream fps_ss;
  fps_ss << "FPS: " << GetFPS();
  DrawText(fps_ss.str().c_str(), screen_width_ - 150, 10, 14, {255, 255, 0, 255});

  EndDrawing();
}

void Renderer::drawIntroScreen(int hole_number, int par, float pin_distance) {
  BeginDrawing();
  ClearBackground({95, 190, 245, 255});

  // Sky gradient & mountains
  for (int y = 0; y < screen_height_ * 0.45f; ++y) {
    float r = (float)y / (screen_height_ * 0.45f);
    Color c = {(unsigned char)(95 + 25 * r), (unsigned char)(190 + 30 * r), (unsigned char)(245 - 40 * r), 255};
    DrawLine(0, y, screen_width_, y, c);
  }
  Vector2 m1[3] = {{screen_width_ * 0.15f, screen_height_ * 0.45f}, {screen_width_ * 0.30f, screen_height_ * 0.20f}, {screen_width_ * 0.45f, screen_height_ * 0.45f}};
  Vector2 m2[3] = {{screen_width_ * 0.40f, screen_height_ * 0.45f}, {screen_width_ * 0.55f, screen_height_ * 0.18f}, {screen_width_ * 0.70f, screen_height_ * 0.45f}};
  Vector2 m3[3] = {{screen_width_ * 0.60f, screen_height_ * 0.45f}, {screen_width_ * 0.78f, screen_height_ * 0.24f}, {screen_width_ * 0.95f, screen_height_ * 0.45f}};
  DrawTriangle(m1[0], m1[1], m1[2], {170, 200, 210, 255});
  DrawTriangle(m2[0], m2[1], m2[2], {190, 210, 220, 255});
  DrawTriangle(m3[0], m3[1], m3[2], {165, 195, 205, 255});

  // Horizon, lake, fairway
  float horizon_y = screen_height_ * 0.48f;
  DrawRectangle(0, horizon_y, screen_width_, screen_height_ - horizon_y, {65, 150, 80, 255});
  DrawRectangle(0, horizon_y - 30, screen_width_, 30, {70, 150, 190, 180}); // water band
  float fair_near_y = horizon_y + 20;
  float fair_far_y = horizon_y - 50;
  Vector2 fair[4] = {
    {screen_width_ * 0.28f, fair_near_y},
    {screen_width_ * 0.72f, fair_near_y},
    {screen_width_ * 0.58f, fair_far_y},
    {screen_width_ * 0.42f, fair_far_y}
  };
  DrawTriangle(fair[0], fair[1], fair[2], {70, 185, 90, 255});
  DrawTriangle(fair[0], fair[2], fair[3], {70, 185, 90, 255});
  DrawLineEx(fair[0], fair[1], 3, {40, 110, 50, 255});
  DrawLineEx(fair[1], fair[2], 3, {40, 110, 50, 255});
  DrawLineEx(fair[2], fair[3], 3, {40, 110, 50, 255});
  DrawLineEx(fair[3], fair[0], 3, {40, 110, 50, 255});

  // Pin flag on distant green
  Vector2 pin = {screen_width_ * 0.50f, fair_far_y - 12};
  DrawLineEx({pin.x, pin.y}, {pin.x, pin.y - 30}, 3, {230, 230, 230, 255});
  DrawTriangle(
    {pin.x, pin.y - 30},
    {pin.x + 18, pin.y - 20},
    {pin.x, pin.y - 10},
    {230, 40, 40, 220}
  );

  // Tee area and ball (foreground)
  Vector2 tee_pos = {screen_width_ * 0.56f, screen_height_ * 0.82f};
  DrawCircle((int)tee_pos.x, (int)tee_pos.y, 7, {210, 170, 100, 255});
  DrawCircle((int)tee_pos.x, (int)tee_pos.y - 9, 9, {255, 255, 255, 255});
  DrawCircleLines((int)tee_pos.x, (int)tee_pos.y - 9, 11, {255, 255, 120, 160});

  // Golfer silhouette on left (back view)
  Vector2 base = {screen_width_ * 0.25f, screen_height_ * 0.82f};
  DrawCircle((int)base.x, (int)(base.y - 42), 13, {30, 30, 30, 230}); // head
  DrawLineEx({base.x, base.y - 30}, {base.x, base.y - 6}, 7, {30, 30, 30, 230}); // torso
  DrawLineEx({base.x, base.y - 6}, {base.x - 12, base.y + 20}, 5, {30, 30, 30, 230}); // left leg
  DrawLineEx({base.x, base.y - 6}, {base.x + 11, base.y + 18}, 5, {30, 30, 30, 230}); // right leg
  DrawLineEx({base.x, base.y - 26}, {base.x - 20, base.y - 10}, 5, {30, 30, 30, 230}); // left arm
  DrawLineEx({base.x, base.y - 26}, {base.x + 20, base.y - 4}, 5, {30, 30, 30, 230}); // right arm
  DrawLineEx({base.x + 20, base.y - 4}, {tee_pos.x - 6, tee_pos.y - 12}, 4, {50, 50, 50, 230}); // club

  // Mini map circle (top-right)
  float mini_r = 48.0f;
  Vector2 mini_c = {screen_width_ - mini_r - 20, 80.0f};
  DrawCircleLines((int)mini_c.x, (int)mini_c.y, (int)mini_r, {40, 110, 50, 200});
  DrawLineEx({mini_c.x - mini_r, mini_c.y}, {mini_c.x + mini_r, mini_c.y}, 1, {120, 180, 120, 120});
  DrawLineEx({mini_c.x, mini_c.y - mini_r}, {mini_c.x, mini_c.y + mini_r}, 1, {120, 180, 120, 120});
  Vector2 mini_pin = {mini_c.x, mini_c.y - mini_r * 0.6f};
  DrawLineEx({mini_pin.x, mini_pin.y}, {mini_pin.x, mini_pin.y - 12}, 2, {230, 230, 230, 255});
  DrawTriangle({mini_pin.x, mini_pin.y - 12}, {mini_pin.x + 10, mini_pin.y - 6}, {mini_pin.x, mini_pin.y}, {230, 40, 40, 220});
  Vector2 mini_ball = {mini_c.x, mini_c.y + mini_r * 0.6f};
  DrawCircle((int)mini_ball.x, (int)mini_ball.y, 4, {255, 255, 255, 255});

  // Wind indicator (top-left)
  float wind_y = 26.0f;
  DrawText("WIND", 20, wind_y, 16, {80, 140, 255, 255});
  DrawLineEx({90, wind_y + 10}, {150, wind_y + 10}, 3, {80, 140, 255, 200});
  DrawTriangle({150, wind_y + 10}, {140, wind_y + 5}, {140, wind_y + 15}, {80, 140, 255, 200});

  // HUD text
  std::stringstream hole_ss; hole_ss << "Hole " << hole_number << "  PAR " << par;
  std::stringstream dist_ss; dist_ss << (int)pin_distance << "y";
  DrawText(hole_ss.str().c_str(), 20, 52, 26, {255, 235, 140, 255});
  DrawText(dist_ss.str().c_str(), 20, 82, 22, {230, 230, 230, 255});

  DrawText("Take your stance...", 20, 118, 20, {230, 230, 230, 255});
  DrawText("SPACE / ENTER to start setup", 20, 146, 18, {180, 240, 180, 255});

  // Bottom power bar placeholder
  float bar_x = screen_width_ * 0.20f;
  float bar_y = screen_height_ - 60;
  float bar_w = screen_width_ * 0.60f;
  DrawRectangle(bar_x, bar_y, bar_w, 18, {30, 30, 30, 200});
  DrawRectangleLines(bar_x, bar_y, bar_w, 18, {200, 200, 200, 200});
  DrawRectangle(bar_x, bar_y, bar_w * 0.35f, 18, {255, 200, 60, 220});
  DrawText("Power", bar_x - 70, bar_y - 2, 16, {255, 235, 140, 255});

  EndDrawing();
}
