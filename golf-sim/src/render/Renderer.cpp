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
    green.current_ball_pos.y * green.current_ball_pos.y
  );
  float size_scale = 0.5f + 0.5f * (17.5f - distance_from_hole) / 17.5f;
  float ball_radius = 10.0f * size_scale;
  
  // Shadow
  DrawCircle((int)ball_pos.x, (int)(ball_pos.y + 3), (int)ball_radius, {50, 50, 50, 100});
  
  // Ball (white with red marking)
  DrawCircle((int)ball_pos.x, (int)ball_pos.y, (int)ball_radius, {255, 255, 255, 255});
  DrawCircle((int)(ball_pos.x - 3), (int)(ball_pos.y - 2), (int)(ball_radius / 4), {255, 100, 100, 255});
  
  // Glow effect
  DrawCircleLines((int)ball_pos.x, (int)ball_pos.y, (int)(ball_radius + 2), {255, 255, 100, 100});
}

void Renderer::drawBalls(const std::vector<BallPosition>& positions) {
  // Draw previous shots (less prominent)
  Color ball_colors[] = {
    {100, 150, 100, 200},
    {50, 100, 150, 200},
    {150, 80, 80, 200},
  };

  for (size_t i = 0; i < positions.size(); ++i) {
    Vector2 screen_pos = mapGreenCoordToScreen(positions[i].x, positions[i].y);
    Color ball_color = ball_colors[i % 3];
    
    DrawCircle((int)screen_pos.x, (int)screen_pos.y, 5, ball_color);
    DrawCircleLines((int)screen_pos.x, (int)screen_pos.y, 5, {50, 50, 50, 150});
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

void Renderer::present() {
  BeginDrawing();
  EndDrawing();
}

void Renderer::drawSetupScreen(float pin_distance, int hole_number, int par,
                               const char* club_name, float wind_speed, float wind_angle) {
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
}
