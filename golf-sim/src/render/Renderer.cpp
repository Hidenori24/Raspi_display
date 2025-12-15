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
  // 1 meter = 30 pixels (approximate scale for good visibility)
  scale_factor_ = 30.0f;
}

void Renderer::drawGreen(const GreenData& green) {
  // Clear background (light sky blue)
  ClearBackground({200, 220, 240, 255});

  // Calculate green center position on screen
  float center_x = screen_width_ / 2.0f;
  float center_y = screen_height_ / 2.0f;

  // Draw green rectangle (light green)
  float green_width_px = green.width * scale_factor_;
  float green_length_px = green.length * scale_factor_;

  DrawRectangle(
    (int)(center_x - green_width_px / 2),
    (int)(center_y - green_length_px / 2),
    (int)green_width_px,
    (int)green_length_px,
    {100, 200, 100, 255}  // Green color
  );

  // Draw green border
  DrawRectangleLines(
    (int)(center_x - green_width_px / 2),
    (int)(center_y - green_length_px / 2),
    (int)green_width_px,
    (int)green_length_px,
    {50, 150, 50, 255}  // Dark green border
  );

  // Draw distance grid (every 5m)
  int grid_interval = 5;
  Color grid_color = {150, 200, 150, 100};
  
  for (float i = -green.length / 2; i <= green.length / 2; i += grid_interval) {
    float y_px = center_y + i * scale_factor_;
    DrawLine(
      (int)(center_x - green_width_px / 2),
      (int)y_px,
      (int)(center_x + green_width_px / 2),
      (int)y_px,
      grid_color
    );
  }
  
  for (float i = -green.width / 2; i <= green.width / 2; i += grid_interval) {
    float x_px = center_x + i * scale_factor_;
    DrawLine(
      (int)x_px,
      (int)(center_y - green_length_px / 2),
      (int)x_px,
      (int)(center_y + green_length_px / 2),
      grid_color
    );
  }

  // Draw center marker
  DrawCircle((int)center_x, (int)center_y, 3, {255, 100, 100, 255});

  // Draw distance labels on grid
  for (float i = 0; i <= green.length / 2; i += 10) {
    if (i > 0) {
      float y_px = center_y - i * scale_factor_;
      std::stringstream ss;
      ss << (int)i << "m";
      DrawText(ss.str().c_str(), (int)(center_x + 10), (int)y_px, 16, {50, 50, 50, 255});
      
      y_px = center_y + i * scale_factor_;
      DrawText(ss.str().c_str(), (int)(center_x + 10), (int)y_px, 16, {50, 50, 50, 255});
    }
  }
}

void Renderer::drawBalls(const std::vector<BallPosition>& positions) {
  float center_x = screen_width_ / 2.0f;
  float center_y = screen_height_ / 2.0f;

  Color ball_colors[] = {
    {255, 200, 0, 255},    // Gold for drive
    {0, 150, 255, 255},    // Blue for 2nd shot
    {255, 100, 100, 255},  // Red for approach
  };

  for (size_t i = 0; i < positions.size(); ++i) {
    float x_px = center_x + positions[i].x * scale_factor_;
    float y_px = center_y + positions[i].y * scale_factor_;

    Color ball_color = ball_colors[i % 3];
    
    // Draw ball
    DrawCircle((int)x_px, (int)y_px, 8, ball_color);
    
    // Draw ball outline
    DrawCircleLines((int)x_px, (int)y_px, 8, {50, 50, 50, 255});

    // Draw shot number
    std::stringstream ss;
    ss << (i + 1);
    DrawText(ss.str().c_str(), (int)(x_px - 5), (int)(y_px - 5), 12, {255, 255, 255, 255});
  }
}

void Renderer::drawHUD(const std::vector<float>& distances) {
  // Draw HUD background (semi-transparent)
  DrawRectangle(10, 10, 300, 150, {0, 0, 0, 100});
  DrawRectangleLines(10, 10, 300, 150, {255, 255, 255, 200});

  // Draw title
  DrawText("Golf Simulator", 20, 20, 20, {255, 255, 255, 255});

  // Draw distance data
  int y_offset = 50;
  for (size_t i = 0; i < distances.size(); ++i) {
    std::stringstream ss;
    ss << "Shot " << (i + 1) << ": " << std::fixed << std::setprecision(1) << distances[i] << "m";
    DrawText(ss.str().c_str(), 20, y_offset + (i * 25), 16, {200, 255, 200, 255});
  }

  // Draw FPS counter
  std::stringstream fps_ss;
  fps_ss << "FPS: " << GetFPS();
  DrawText(fps_ss.str().c_str(), screen_width_ - 150, 10, 16, {255, 255, 0, 255});
}

void Renderer::present() {
  BeginDrawing();
  EndDrawing();
}
