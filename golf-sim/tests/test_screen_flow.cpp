#include "application/ScreenFlow.hpp"
#include "domain/GameState.hpp"
#include <iostream>
#include <cassert>

// Simple test framework (same as test_physics)
#define TEST(name) void test_##name()
#define RUN_TEST(name) do { \
  std::cout << "Running " #name "..." << std::endl; \
  test_##name(); \
  std::cout << "  PASSED" << std::endl; \
} while(0)

TEST(transition_intro_to_playing_enters_cinematic) {
  application::ScreenFlow flow;
  // initial
  assert(flow.screenState() == application::ScreenFlow::ScreenState::Intro);
  assert(!flow.cinematicEnabled());
  // advance
  bool advanced = flow.advanceFromIntro();
  assert(advanced);
  assert(flow.screenState() == application::ScreenFlow::ScreenState::Playing);
  assert(flow.cinematicEnabled());
  // enter Armed should keep cinematic on (user starts in silhouette)
  flow.onGameStateChange(domain::GameState::Armed);
  assert(flow.cinematicEnabled());
}

TEST(toggle_and_shot_forces_overhead) {
  application::ScreenFlow flow;
  flow.advanceFromIntro();
  flow.onGameStateChange(domain::GameState::Armed);
  // start in cinematic; shot forces overhead
  flow.onShot();
  assert(!flow.cinematicEnabled());
  // entering flight keeps overhead
  flow.onGameStateChange(domain::GameState::InFlight);
  assert(!flow.cinematicEnabled());
}

TEST(result_resets_to_intro) {
  application::ScreenFlow flow;
  flow.advanceFromIntro();
  flow.onGameStateChange(domain::GameState::Armed);
  flow.onGameStateChange(domain::GameState::InFlight);
  flow.onGameStateChange(domain::GameState::Result);
  flow.toggleCinematic();
  assert(flow.cinematicEnabled());
  flow.onNextHole();
  assert(flow.screenState() == application::ScreenFlow::ScreenState::Intro);
  assert(!flow.cinematicEnabled());
}

TEST(view_selection_respects_cinematic) {
  application::ScreenFlow flow;
  // Initially in Intro, cinematic is off -> overhead
  assert(flow.selectView(domain::GameState::Idle) == ViewMode::OverheadView);
  
  // After advancing to Playing, cinematic is on -> player view
  flow.advanceFromIntro();
  assert(flow.cinematicEnabled());
  assert(flow.selectView(domain::GameState::Armed) == ViewMode::PlayerView);
  
  // User can toggle cinematic off in Armed state
  flow.toggleCinematic();
  assert(!flow.cinematicEnabled());
  assert(flow.selectView(domain::GameState::Armed) == ViewMode::OverheadView);
  
  // Toggle back on
  flow.toggleCinematic();
  assert(flow.cinematicEnabled());
  assert(flow.selectView(domain::GameState::Armed) == ViewMode::PlayerView);
  
  // After shot, cinematic is forced off -> overhead
  flow.onShot();
  assert(!flow.cinematicEnabled());
  assert(flow.selectView(domain::GameState::InFlight) == ViewMode::OverheadView);
  
  // During InFlight, always overhead (even if flag is forced, which shouldn't happen)
  flow.onGameStateChange(domain::GameState::InFlight);
  assert(!flow.cinematicEnabled());
  assert(flow.selectView(domain::GameState::InFlight) == ViewMode::OverheadView);
  
  // During Result, always overhead
  flow.onGameStateChange(domain::GameState::Result);
  assert(!flow.cinematicEnabled());
  assert(flow.selectView(domain::GameState::Result) == ViewMode::OverheadView);
}

int main() {
  std::cout << "=== Screen Flow Tests ===" << std::endl;
  RUN_TEST(transition_intro_to_playing_enters_cinematic);
  RUN_TEST(toggle_and_shot_forces_overhead);
  RUN_TEST(result_resets_to_intro);
  RUN_TEST(view_selection_respects_cinematic);
  std::cout << "All screen flow tests passed!" << std::endl;
  return 0;
}
