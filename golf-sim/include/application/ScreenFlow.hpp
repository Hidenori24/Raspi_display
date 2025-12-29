// ScreenFlow: testable controller for screen/view transitions (no raylib dependency)
#pragma once

#include "domain/GameState.hpp"
#include "render/Renderer.hpp" // for ViewMode

namespace application {

class ScreenFlow {
public:
  ScreenFlow();

  // Screen states used by App
  enum class ScreenState { Intro, Playing };

  // Advance from intro to playing (returns true if transition happened)
  bool advanceFromIntro();

  // Toggle cinematic (golfer silhouette)
  void toggleCinematic();

  // Called when a shot is executed (forces overhead)
  void onShot();

  // Called when game state changes (Armed/InFlight/Result)
  void onGameStateChange(domain::GameState new_state);

  // Called when advancing to next hole (Result -> Intro)
  void onNextHole();

  // Select view mode for renderer (currently only overhead for stability)
  ViewMode selectView(domain::GameState state) const;

  // Accessors for App integration/UI
  ScreenState screenState() const { return screen_state_; }
  bool cinematicEnabled() const { return cinematic_view_; }

  // Reset helpers
  void resetToIntro();

private:
  ScreenState screen_state_ = ScreenState::Intro;
  bool cinematic_view_ = false;
  domain::GameState last_state_ = domain::GameState::Idle;
};

} // namespace application
