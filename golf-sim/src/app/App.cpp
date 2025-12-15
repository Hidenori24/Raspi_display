#include "app/App.hpp"
#include "render/Renderer.hpp"
#include <raylib.h>
#include <iostream>

App::App() {
  setup();
}

App::~App() {
  CloseWindow();
}

void App::setup() {
  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Raspberry Pi 5 - Golf Simulator");
  SetTargetFPS(60);
}

void App::run() {
  Renderer renderer;
  renderer.init(SCREEN_WIDTH, SCREEN_HEIGHT);

  // Sample green data
  GreenData green;
  green.width = 20.0f;   // 20m wide
  green.length = 35.0f;  // 35m long
  
  // Sample ball positions (x, y in meters, relative to green center)
  green.ball_positions = {
    {-5.0f, -10.0f},  // drive
    {2.0f, 3.0f},     // 2nd shot
    {-1.0f, -0.5f},   // approach
  };
  
  green.distances_m = {210.4f, 150.2f, 8.5f};

  while (!WindowShouldClose()) {
    update();
    draw();
    renderer.drawGreen(green);
    renderer.drawBalls(green.ball_positions);
    renderer.drawHUD(green.distances_m);
    renderer.present();
  }
}

bool App::shouldClose() const {
  return window_open_;
}

void App::update() {
  // TODO: Update game state
}

void App::draw() {
  // Renderer handles drawing
}
