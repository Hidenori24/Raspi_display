#pragma once

#include <vector>
#include <glm/glm.hpp>

struct BallPosition {
  float x, y;  // 2D position on green
};

struct GreenData {
  float width;
  float length;
  std::vector<BallPosition> ball_positions;
  std::vector<float> distances_m;  // corresponding distances
};

class Renderer {
public:
  Renderer();
  ~Renderer();

  void init(int width, int height);
  void drawGreen(const GreenData& green);
  void drawBalls(const std::vector<BallPosition>& positions);
  void drawHUD(const std::vector<float>& distances);
  void present();

private:
  int screen_width_ = 1280;
  int screen_height_ = 720;
  float scale_factor_ = 1.0f;  // pixels per meter
};
