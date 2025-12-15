#pragma once

class App {
public:
  App();
  ~App();

  void run();
  bool shouldClose() const;

private:
  void setup();
  void update();
  void draw();

  bool window_open_ = true;
  static const int SCREEN_WIDTH = 1280;
  static const int SCREEN_HEIGHT = 720;
};
