#pragma once

enum class GamePhase {
  Setup,      // 打つ前の準備画面
  Aiming,     // 狙う画面
  InFlight,   // ボール飛行中
  Result      // 結果表示
};

struct ClubData {
  const char* name;
  float distance_avg;  // Average carry distance
  float speed;         // m/s
  float launch_angle;  // degrees
  float spin;          // rpm
};

struct GameStateData {
  GamePhase phase = GamePhase::Setup;
  int hole_number = 1;
  int current_par = 4;
  float pin_distance = 249.0f;
  float wind_speed = 5.0f;
  float wind_direction = 45.0f;  // degrees
  
  // Club selection and data
  int selected_club = 0;  // 0=Driver, 1=3Wood, etc.
  static constexpr int num_clubs = 6;
  static constexpr ClubData clubs[] = {
    {"Driver",   250.0f, 68.0f, 12.0f, 2500.0f},
    {"3-Wood",   210.0f, 55.0f, 15.0f, 3500.0f},
    {"5-Iron",   180.0f, 48.0f, 18.0f, 4000.0f},
    {"7-Iron",   155.0f, 42.0f, 21.0f, 4500.0f},
    {"9-Iron",   130.0f, 38.0f, 24.0f, 5000.0f},
    {"Putter",    3.0f,  2.0f, 0.0f,    0.0f},
  };
  
  // Random hole position
  float hole_x = 0.0f;   // -10 to +10
  float hole_y = 17.5f;  // Near the far end
  
  // Shot parameters
  float power = 0.5f;  // 0.0 to 1.0
  float aim_angle = 0.0f;  // degrees
  
  // Stats
  int total_strokes = 0;
  float total_distance = 0.0f;
};
