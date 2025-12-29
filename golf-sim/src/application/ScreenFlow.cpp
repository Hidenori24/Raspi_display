#include "application/ScreenFlow.hpp"

namespace application {

ScreenFlow::ScreenFlow() {
  resetToIntro();
}

bool ScreenFlow::advanceFromIntro() {
  if (screen_state_ == ScreenState::Intro) {
    screen_state_ = ScreenState::Playing;
    cinematic_view_ = true;  // start with silhouette view
    return true;
  }
  return false;
}

void ScreenFlow::toggleCinematic() {
  cinematic_view_ = !cinematic_view_;
}

void ScreenFlow::onShot() {
  // Enforce overhead during flight
  cinematic_view_ = false;
}

void ScreenFlow::onGameStateChange(domain::GameState new_state) {
  if (new_state != last_state_) {
    // InFlight/Result should always be overhead (no cinematic)
    if (new_state == domain::GameState::InFlight || new_state == domain::GameState::Result) {
      cinematic_view_ = false;
    }
    last_state_ = new_state;
  }
}

void ScreenFlow::onNextHole() {
  resetToIntro();
}

ViewMode ScreenFlow::selectView(domain::GameState /*state*/) const {
  // Spec: only two views: cinematic (golfer silhouette) or overhead
  // Renderer still switches perspective; we choose overhead when cinematic is off.
  return ViewMode::OverheadView;
}

void ScreenFlow::resetToIntro() {
  screen_state_ = ScreenState::Intro;
  cinematic_view_ = false;
  last_state_ = domain::GameState::Idle;
}

} // namespace application
