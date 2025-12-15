# Golf Simulator - Refactored Architecture

## Overview
This codebase implements a golf simulator for Raspberry Pi 5 using **Clean Architecture** and **Domain-Driven Design (DDD)** principles. The architecture prioritizes testability, determinism, and layer separation.

## Layer Structure

### 1. Domain Layer (Pure C++)
**Location**: `include/domain/`, `src/domain/`  
**Dependencies**: NONE (pure C++17)

**Contains**:
- **Entities**: `BallState`, `Trajectory`
- **Value Objects**: `Vec3`, `LaunchCondition`, `ShotResult`
- **Domain Services**: `PhysicsEngine`, `GameStateMachine`
- **Domain State**: `GameState` enum

**Constraints**:
- ❌ NO I/O operations (file, network, sensor)
- ❌ NO raylib or graphics dependencies
- ❌ NO JSON/CSV parsing
- ❌ NO time functions (`std::chrono`, `time()`)
- ❌ NO threads or async operations
- ❌ NO non-deterministic random numbers
- ✅ Pure business logic only
- ✅ Deterministic physics with fixed timestep

**Key Classes**:
- `PhysicsEngine`: Fixed timestep (1/240 sec) ballistic simulation with gravity + drag + wind
- `GameStateMachine`: State transitions (Idle → Armed → InFlight → Result)
- `Trajectory`: Collection of `BallState` points during flight

### 2. Application Layer
**Location**: `include/application/`, `src/application/`  
**Dependencies**: Domain

**Contains**:
- **Use Cases**: `ExecuteShotUseCase`, `UpdatePhysicsUseCase`
- **Application Services**: `ShotParameterService`
- **Application DTOs**: `ShotParameters`, `ClubData`

**Responsibilities**:
- Orchestrate domain logic for user workflows
- Translate UI parameters to domain concepts
- Bridge between input events and domain commands

**Key Classes**:
- `ShotParameterService`: Converts club selection + power + aim → `LaunchCondition`
- `ExecuteShotUseCase`: Coordinates shot execution through state machine and physics
- `UpdatePhysicsUseCase`: Updates physics and checks for landing

### 3. Infrastructure Layer
**Location**: `include/infrastructure/`, `src/infrastructure/`  
**Dependencies**: Application, Domain

**Contains**:
- **Sensor Providers**: `ISensorProvider`, `MockSensorProvider`, (future: `ReplaySensorProvider`, `RealSensorProvider`)
- **Configuration**: (future: JSON config loading)
- **Logging**: (future: CSV/JSON event logging)

**Responsibilities**:
- Provide sensor input abstraction
- Handle I/O operations (files, hardware)
- Mock/Replay infrastructure for testing

**Key Classes**:
- `ISensorProvider`: Strategy interface for sensor input
- `MockSensorProvider`: Deterministic mock with seed-controlled randomness
- `SensorFrame`: Raw sensor data structure (accelerometer + gyro)

### 4. Presentation Layer
**Location**: `include/app/`, `include/render/`, `src/app/`, `src/render/`  
**Dependencies**: Application, Infrastructure, Domain, **raylib**

**Contains**:
- **Main App**: `App` (composition root)
- **Rendering**: `Renderer` (raylib-dependent)
- **UI State**: View models and presentation logic

**Responsibilities**:
- Handle user input (keyboard)
- Render graphics (raylib)
- Manage UI state and screens
- Assemble all layers (dependency injection)

**Key Classes**:
- `App`: Composition root, assembles all dependencies, main loop
- `Renderer`: Draws green, trajectory, HUD (raylib-only)

## Dependency Flow
```
┌─────────────────────────────────────┐
│        Presentation Layer           │
│  (App, Renderer - raylib)          │
└───────────┬─────────────────────────┘
            │
            ↓
┌─────────────────────────────────────┐
│      Infrastructure Layer           │
│  (MockSensor, Config, Logger)      │
└───────────┬─────────────────────────┘
            │
            ↓
┌─────────────────────────────────────┐
│       Application Layer             │
│  (UseCases, Services)              │
└───────────┬─────────────────────────┘
            │
            ↓
┌─────────────────────────────────────┐
│         Domain Layer                │
│  (Physics, Entities - PURE)        │
└─────────────────────────────────────┘
```

## Build System
CMake enforces layer boundaries through separate static libraries:

```cmake
domain         (STATIC, no dependencies)
application    (STATIC, depends on domain)
infrastructure (STATIC, depends on application, domain)
presentation   (STATIC, depends on all + raylib)
golf-sim       (EXECUTABLE, composition root)
```

**Building**:
```bash
cd golf-sim
mkdir build && cd build
cmake ..
make -j4
```

**Testing**:
```bash
./tests/test_physics  # Domain layer tests
```

## Testing Strategy

### Unit Tests (Domain)
- **Purpose**: Validate physics correctness and determinism
- **Location**: `tests/test_physics.cpp`
- **Tests**:
  - `physics_determinism`: Same input → same output (fixed seed)
  - `physics_gravity_only`: Verify ballistic equations
  - `physics_drag_reduces_distance`: Drag effect validation
  - `physics_trajectory_points`: Trajectory recording

**All domain tests must pass before merging code.**

### Integration Tests (Future)
- Replay tests: Load recorded sensor traces → verify identical results
- Mock scenarios: Predefined shot types with expected outcomes

## Design Patterns Used

### Strategy Pattern
- `ISensorProvider`: Switch between Real/Mock/Replay sensors
- Future: `IBallisticsModel` for different physics models

### State Pattern
- `GameStateMachine`: Explicit state transitions with invariants

### Factory Pattern
- Composition Root in `App::App()`: Creates and wires dependencies

### Adapter Pattern
- `ShotParameterService`: Translates UI params → Domain concepts

## Key Design Decisions

### 1. Fixed Timestep Physics
- **Rationale**: Ensures deterministic simulation regardless of frame rate
- **Implementation**: Accumulator pattern in `PhysicsEngine::step()`
- **Benefit**: Replay tests produce identical results

### 2. Pure Domain Layer
- **Rationale**: Enable thorough unit testing without mocks
- **Implementation**: Zero I/O, no external libs in domain
- **Benefit**: Fast tests, easy debugging, portable code

### 3. Interface-Based Sensor Input
- **Rationale**: Support Mock/Replay/Real without changing domain
- **Implementation**: `ISensorProvider` with non-blocking `poll()`
- **Benefit**: Develop without hardware, record bugs for replay

### 4. Dependency Injection at Root
- **Rationale**: Avoid global state and singletons
- **Implementation**: `App` constructor wires all dependencies
- **Benefit**: Testable composition, clear ownership

## Code Quality Rules

### Domain Layer ✅
- ✅ Pure C++17, no external dependencies
- ✅ All public methods have unit tests
- ✅ Fixed timestep for determinism
- ✅ No I/O, time, or random operations

### All Layers ✅
- ✅ Use `unique_ptr` for ownership (avoid `shared_ptr` unless justified)
- ✅ No global variables or singletons
- ✅ Include guard: `#pragma once`
- ✅ Forward declarations to minimize includes
- ✅ Const correctness

## Future Work
- [ ] Add `ReplaySensorProvider` for CSV trace playback
- [ ] Implement `EventQueue` with drop metrics
- [ ] Add JSON configuration loading
- [ ] Add CSV/JSON event logging
- [ ] Create CLI arguments for sensor selection
- [ ] Add `--headless` mode for CI
- [ ] Implement Magnus force for spin effects
- [ ] Add integration tests with recorded traces

## References
- Design Doc: `doc/raspi5_golf_sim_design_doc.md`
- Architecture Diagram: `doc/raspi5_golf_sim_architecture.svg`
- Instructions: `.github/copilot-instructions.md`
