#pragma once

enum class GamePhase {
  Setup,      // 打つ前の準備画面
  Aiming,     // 狙う画面
  InFlight,   // ボール飛行中
  Result      // 結果表示
};

struct GameStateData {
  GamePhase phase = GamePhase::Setup;
  int hole_number = 1;
  int current_par = 4;
  float pin_distance = 249.0f;
  float wind_speed = 5.0f;
  float wind_direction = 45.0f;  // degrees
  
  // Club selection
  int selected_club = 0;  // 0=Driver, 1=Iron7, etc.
  static constexpr const char* club_names[] = {"Driver", "3-Wood", "5-Iron", "7-Iron", "9-Iron", "Putter"};
  static constexpr int num_clubs = 6;
  
  // Shot parameters
  float power = 0.5f;  // 0.0 to 1.0
  float aim_angle = 0.0f;  // degrees
  
  // Stats
  int total_strokes = 0;
  float total_distance = 0.0f;
};
