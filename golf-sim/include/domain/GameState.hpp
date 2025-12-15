#pragma once

namespace domain {

// Domain state machine states
enum class GameState {
  Idle,       // Waiting to start
  Armed,      // Ready for shot (club selected, aimed)
  InFlight,   // Ball is in flight
  Result      // Showing result
};

// Convert state to string for debugging/logging
const char* gameStateToString(GameState state);

} // namespace domain
