# Raspberry Pi 5 ゴルフシミュレータ（プロトタイプ）設計書（ドラフト）

本書は、Raspberry Pi 5 上で「ゴルフシミュレータもどき」を **安定した描画**と **テスト容易性（Mock/Replay）** を両立して実装するための推奨アーキテクチャと実装方針をまとめたものです。

---

## 1. 目的とスコープ

### 1.1 目的
- Raspberry Pi 5 上で、スイング入力（当面はダミー/Mock）から **弾道計算→表示**までを実現する。
- 実センサー導入前に、**Mock / Record-Replay** で挙動検証・回帰試験が可能な構造にする。
- 60FPS を基本ターゲット（最低でも 30FPS 安定）とし、フレーム落ちを起こしにくい設計にする。

### 1.2 非スコープ（当面やらない）
- Unity/Unreal 等の重量級 3D エンジン
- 高精度な流体計算／公式弾道モデルの完全再現
- ネットワーク対戦・クラウド連携（後で拡張可能）

---

## 2. 要件

### 2.1 機能要件
- 練習モード：1 球打つ→弾道表示→結果表示（carry/total/左右ブレ）
- 設定：クラブプロファイル、風、重力、係数（JSON）
- ログ：入力イベントと結果（CSV/JSON）を保存
- デバッグ：FPS、入力イベント、現在状態をオーバレイ表示

### 2.2 非機能要件
- フレームループ（描画/更新）を **安定**させる
- センサー入力は **可変レート**でも受けられる（イベントキューで吸収）
- 実行時依存（センサー有無）を排除（インタフェースで注入）

---

## 3. 推奨アーキテクチャ概要

添付 SVG（`raspi5_golf_sim_architecture.svg`）の通り、以下の分離を基本とします。
![svgの概念図](raspi5_golf_sim_architecture.svg)
- **Input Adapter**: センサー入力を抽象化し、イベントに変換
- **Event Bus**: 可変レートの入力をキューイングして、描画ループと分離
- **Game State**: 状態遷移（Practice / Round / Results）とコマンド生成
- **Physics Engine**: 決定論的 timestep で弾道更新（テストしやすい）
- **Renderer/UI**: raylib で 2D/疑似3D 表示

---

## 4. モジュール構成（C++）

### 4.1 ディレクトリ案
```
golf-sim/
  CMakeLists.txt
  assets/
    course/...
    fonts/...
  config/
    clubs.json
    sim.json
  include/
    app/App.hpp
    input/ISensorProvider.hpp
    input/MockSensorProvider.hpp
    input/ReplaySensorProvider.hpp
    input/RealSensorProvider.hpp
    core/Event.hpp
    core/EventQueue.hpp
    game/GameStateMachine.hpp
    physics/PhysicsEngine.hpp
    render/Renderer.hpp
    util/Config.hpp
    util/Logger.hpp
    util/Time.hpp
  src/
    main.cpp
    ... (対応実装)
  tests/
    test_physics.cpp
    test_replay.cpp
```

### 4.2 主要インタフェース

#### ISensorProvider（差し替え点）
```cpp
struct SensorFrame {
  double t_sec;        // monotonic time
  float ax, ay, az;    // accel
  float gx, gy, gz;    // gyro
  // 必要なら姿勢(quat)等を追加
};

class ISensorProvider {
public:
  virtual ~ISensorProvider() = default;
  virtual bool poll(SensorFrame& out) = 0; // non-blocking
};
```

#### Event（アプリ内部の共通言語）
```cpp
enum class EventType { Pose, SwingStart, Impact, SwingEnd };

struct Event {
  EventType type;
  double t_sec;
  // payload (variantにしてもよい)
  SensorFrame frame;
};
```

#### EventQueue（入力とフレームループの分離）
- 実装はリングバッファ（単一プロデューサ/単一コンシューマ）を推奨
- オーバーフロー時は「最新優先」もしくは「ドロップ率ログ」

---

## 5. データフロー（表示まで）

1. `ISensorProvider::poll()` がセンサ（または Mock/Replay）からフレーム取得
2. Input Adapter が **正規化・フィルタ**し、`Event` に変換して `EventQueue` へ push
3. メインループ（60FPS）が `EventQueue` からイベントを drain
4. `GameStateMachine` がイベントを解釈し、`PhysicsEngine` にコマンド（例：ShotStart）を発行
5. `PhysicsEngine` が固定 timestep（例：1/240s）でボール状態を更新
6. `Renderer` が現在状態と軌跡を描画して HDMI へ表示

---

## 6. Mock / Record-Replay 設計

### 6.1 Mock（開発初期の主力）
- `MockSensorProvider` で以下を提供
  - 定常姿勢（アドレス）
  - スイング開始→インパクトの擬似データ（シナリオ駆動）
  - ノイズ付与（再現性のため seed 固定）

例：`--sensor=mock --scenario=driver` のように CLI で切替。

### 6.2 Record（実機導入後の回帰試験）
- 実センサーから取得した `SensorFrame` を CSV/JSON に保存
- 保存形式例（CSV）
  - `t,ax,ay,az,gx,gy,gz`

### 6.3 Replay（最重要：バグ再現と性能評価）
- `ReplaySensorProvider` はトレースを読み、時間に合わせてフレームを供給
- これにより **同じ入力で同じ結果**（決定論的）を確認できる

---

## 7. メインループ（擬似コード）

```cpp
auto sensor = MakeSensorProvider(cfg); // mock/real/replay
EventQueue q;
GameStateMachine gsm;
PhysicsEngine phys(simCfg);
Renderer r;

while (!r.shouldClose()) {
  // 1) Input polling (non-blocking)
  SensorFrame f;
  while (sensor->poll(f)) {
    auto ev = inputAdapter.toEvent(f); // Pose/Impact...
    q.push(ev);
  }

  // 2) Drain events and update game logic
  Event ev;
  while (q.pop(ev)) {
    gsm.onEvent(ev, phys); // may trigger shot start/impact etc.
    logger.write(ev);
  }

  // 3) Fixed timestep physics
  phys.stepFixed(); // e.g., 240Hz internal accumulator

  // 4) Render at display rate
  r.draw(gsm, phys);
}
```

ポイント：
- **poll と描画を分離**し、可変レート入力がフレームを壊さないようにする
- 物理は **固定 timestep**で回す（テストが簡単）

---

## 8. 表示（raylib 前提）

### 8.1 表示要素（最小）
- コース（2D top view で十分）
- ボール位置、過去軌跡（polyline）
- HUD（carry/total/左右/風/クラブ）

### 8.2 性能の勘所
- 文字描画・軌跡点数が増えると重いので、軌跡は間引き（例：最大 2000 点）
- 計算結果を毎フレーム全再計算しない（状態を持つ）

---

## 9. ビルドと実行（例）

### 9.1 依存
- raylib
- Eigen（任意だが推奨）
- Catch2（テスト用、任意）

### 9.2 CMake（方針）
- `-O3`（Release）
- Pi 上は OpenGL ES の設定に注意（環境により差異あり）

実行例：
- `./golf-sim --sensor=mock --scenario=basic`
- `./golf-sim --sensor=replay --trace=traces/swing01.csv`

---

## 10. テスト戦略

### 10.1 単体テスト（Physics）
- 入力（初速/角度/風/係数）→ 出力（carry 等）が期待範囲に収まる
- 乱数がある場合は seed 固定

### 10.2 結合テスト（Replay）
- 既知トレース → 結果（距離/左右）がスナップショット一致
- FPS・処理時間のメトリクスも同時記録（性能退行を検知）

---

## 11. 今後の拡張ポイント
- 実センサー（IMU）導入：RealSensorProvider を実装して差し替え
- カメラ導入：別スレッドで推定→イベント化（EventBus に投入）
- 表示強化：疑似3D（地平線、奥行スケーリング）でも十分見栄えが出る

---

## 付録：実装優先順位（おすすめ）
1. mock 入力 → 物理 → 2D 表示（最短で「動く」）
2. 設定 JSON / ログ / リプレイ
3. 状態遷移（練習/ラウンド）
4. 実センサー導入（最後）
