#pragma once

#include <vector>
#include <raylib.h>

struct BallPosition {
  float x, y;  // 2D position on green
};

struct TrajectoryPoint {
  float x, y;      // 2D position
  float height;    // Height above ground (for drawing arc)
};

struct GreenData {
  float width;
  float length;
  std::vector<BallPosition> ball_positions;
  std::vector<float> distances_m;  // corresponding distances
  std::vector<TrajectoryPoint> trajectory;  // Current ball trajectory
  BallPosition current_ball_pos;  // Current ball position
  float carry_distance = 0.0f;
  float lateral_distance = 0.0f;
};

class Renderer {
public:
  Renderer();
  ~Renderer();

  void init(int width, int height);
  void drawGreen(const GreenData& green);
  void drawTrajectory(const GreenData& green);  // Draw ball flight path
  void drawBalls(const std::vector<BallPosition>& positions);
  void drawCurrentBall(const GreenData& green);  // Draw in-flight ball
  void drawHUD(const GreenData& green);
  void drawSetupScreen(float pin_distance, int hole_number, int par, 
                       const char* club_name, float wind_speed, float wind_angle);
  void drawSetupScreenWithGreen(float pin_distance, int hole_number, int par, 
                                const char* club_name, float wind_speed, float wind_angle,
                                const GreenData& green);
  void drawIntroScreen(int hole_number, int par, float pin_distance);

private:
  int screen_width_ = 1280;
  int screen_height_ = 720;
  float scale_factor_ = 1.0f;  // pixels per meter
  
  // Perspective mapping helpers
  struct PerspectiveParams {
    float vanish_x;
    float vanish_y;
    float near_y;
    float far_y;
    float near_width;
    float far_width;
    float perspective_scale;
  } persp_;
  
  void updatePerspectiveParams();
  Vector2 mapGreenCoordToScreen(float green_x, float green_y);
};
