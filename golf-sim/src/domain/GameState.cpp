#include "domain/GameState.hpp"

namespace domain {

const char* gameStateToString(GameState state) {
  switch (state) {
    case GameState::Idle: return "Idle";
    case GameState::Armed: return "Armed";
    case GameState::InFlight: return "InFlight";
    case GameState::Result: return "Result";
    default: return "Unknown";
  }
}

} // namespace domain
