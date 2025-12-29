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

// Test: selectView currently returns OverheadView for stability
// This is intentional - the cinematic flag controls UI state, but
// selectView always returns OverheadView until PlayerView is stable.
TEST(view_selection_always_overhead_for_stability) {
  application::ScreenFlow flow;
  flow.advanceFromIntro();
  
  // Even with cinematic enabled (golfer silhouette UI state)
  assert(flow.cinematicEnabled());
  assert(flow.selectView(domain::GameState::Armed) == ViewMode::OverheadView);
  
  // And with cinematic disabled
  flow.onShot();
  assert(!flow.cinematicEnabled());
  flow.onGameStateChange(domain::GameState::InFlight);
  assert(flow.selectView(domain::GameState::InFlight) == ViewMode::OverheadView);
  
  // This behavior is temporary - see test below for expected future behavior
}

// FUTURE: When PlayerView is enabled, selectView should respect cinematic flag
// TEST(view_selection_respects_cinematic_flag) {
//   application::ScreenFlow flow;
//   flow.advanceFromIntro();
//   
//   // When cinematic is enabled, should use PlayerView for Armed state
//   assert(flow.cinematicEnabled());
//   assert(flow.selectView(domain::GameState::Armed) == ViewMode::PlayerView);
//   
//   // After shot, cinematic is disabled, should use OverheadView
//   flow.onShot();
//   assert(!flow.cinematicEnabled());
//   assert(flow.selectView(domain::GameState::Armed) == ViewMode::OverheadView);
//   
//   // InFlight and Result always use OverheadView regardless of cinematic
//   flow.toggleCinematic();  // turn cinematic back on
//   assert(flow.cinematicEnabled());
//   flow.onGameStateChange(domain::GameState::InFlight);
//   assert(flow.selectView(domain::GameState::InFlight) == ViewMode::OverheadView);
// }

int main() {
  std::cout << "=== Screen Flow Tests ===" << std::endl;
  RUN_TEST(transition_intro_to_playing_enters_cinematic);
  RUN_TEST(toggle_and_shot_forces_overhead);
  RUN_TEST(result_resets_to_intro);
  RUN_TEST(view_selection_always_overhead_for_stability);
  std::cout << "All screen flow tests passed!" << std::endl;
  return 0;
}
