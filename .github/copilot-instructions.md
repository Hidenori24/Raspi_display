# .github/copilot-instructions.md（日本語版）

## 0. このリポジトリについて
本リポジトリは Raspberry Pi 5 上で動作する「ゴルフシミュレータ（プロトタイプ）」を、C++で **構造化（DDD / Clean Architecture / OOP）** して開発する。  
Copilot はコード生成・編集時に、**設計の境界とテスト容易性**を最優先すること。

### ディレクトリ構成メモ（生成時の参照用）
```
Raspi_display/
├─ README.md
├─ doc/
│  ├─ raspi5_golf_sim_architecture.svg
│  └─ raspi5_golf_sim_design_doc.md
└─ golf-sim/
	├─ CMakeLists.txt
	├─ README.md
	├─ include/
	│  ├─ app/
	│  ├─ game/
	│  └─ render/
	├─ src/
	│  ├─ app/
	│  ├─ render/
	│  └─ main.cpp
	└─ build/   # 生成物
```

---

## 1. 最優先原則（Non-negotiable）
1. **Domainは純粋**：Domain層に I/O（ファイル/ネットワーク/センサー）、raylib、スレッド、sleep、現在時刻、非決定乱数を入れない。
2. **依存方向は外→内のみ**：Presentation/Infrastructure → Application → Domain を絶対に破らない。
3. **差し替え可能性**：Real/Mock/Replay は interface と factory/config で切替できる構造にする（ドメイン改修で切替しない）。
4. **決定論**：物理は固定dt、Mockはseed固定、Replayは同一入力で同一結果（許容誤差内）。
5. **テスト必須**：Domainロジック追加・変更には単体テストを付ける（例外は理由を明記）。

---

## 2. レイヤ責務（境界を壊さない）
### 2.1 Domain（最内側）
**含む**：Entity / Value Object / Domain Service / Domain Event / 不変条件  
**禁止**：I/O、raylib、JSON/CSVパース、OS API、時刻取得、スレッド

### 2.2 Application
**含む**：UseCase、CommandHandler、ワークフロー、InputEvent→Command→DomainEvent の橋渡し  
**禁止**：raylib詳細、センサー実装、CSV/JSON直叩き

### 2.3 Infrastructure
**含む**：ISensorProvider(Real/Mock/Replay)、Record/Replay、Config/Logger、I/O実装  
**禁止**：ドメインルールそのものをここで実装しない（呼び出すだけ）

### 2.4 Presentation
**含む**：Renderer/HUD/UI入力、raylib依存、描画フレームループ  
**禁止**：物理計算、検出ロジック、ドメイン不変条件の実装

---

## 3. ユビキタス言語（命名規約）
ドメイン語彙を優先して命名する。

推奨語彙：
- Shot / Impact / LaunchCondition / BallState / Trajectory / ShotResult
- GameMode / GameState / DomainEvent / Command
- SensorFrame（生）/ InputEvent（正規化後）

Domain内部に UI 用語（Screen/Sprite等）を持ち込まない。

---

## 4. パターン採用指針（使いどころを明確に）
- Strategy：ISensorProvider（Real/Mock/Replay）、ImpactDetector、BallisticsModel
- State：GameStateMachine（Idle/Armed/InFlight/Result）
- Factory：設定/CLIから生成する箇所（ProviderFactory 等）
- Adapter：SensorFrame→InputEvent 変換
- Observer/EventBus：イベント疎結合（ただしやり過ぎない）

「パターンを使うために作る」は禁止。差し替え・境界維持・テスト容易性に効く場合だけ導入。

---

## 5. Event と Command を混ぜない（重要）
- Command：意図（StartShotCommand など）
- Event：事実（ImpactDetected / ShotLanded など）

Inputは InputEvent を出す。  
Applicationが InputEvent を Command に変換する。  
Domainが DomainEvent を発行する。

---

## 6. C++実装規約（構造崩壊を防ぐ）
- 所有権：原則 `unique_ptr`。`shared_ptr` は最終手段（理由を明記）
- グローバル/シングルトン禁止。DIは Composition Root（App組立）で行う
- include最小化、前方宣言を優先
- raylib include は Presentation 限定
- JSON/CSVライブラリ include は Infrastructure 限定
- Domainは原則例外で通常フロー制御しない

---

## 7. ビルド（境界をCMakeで守る）
ターゲット分割で境界をビルドで強制する。

例：
- domain（STATIC）
- application（STATIC）→ domain
- infrastructure（STATIC）→ application/domain
- presentation（STATIC）→ application/domain（raylibはここ）
- app（EXECUTABLE）→ 全部を組み立て

Domainがraylib/JSON/CSVに触れたくなったら設計が間違い。

---

## 8. テスト規約
- Domain（物理/ルール）は単体テスト必須（決定論）
- Replayは結合テストの主役（既知トレース→結果一致/許容誤差）
- 可能なら `--headless` 実行を用意してCIに載せられる構造にする

---

## 9. Copilotが出すべきアウトプットの形
- 小さく凝集したクラス（SRP）
- インタフェースで差し替え可能
- レイヤ配置が明確（どこに置くか説明できる）
- 物理は固定dt、Mock/Replayで再現できる
- 変更にテストが付く

---

## 10. PRチェック（提案時に必ず満たす）
- [ ] Domainは純粋（I/Oなし、raylibなし、時刻/スレッドなし）
- [ ] 依存方向は外→内
- [ ] Real/Mock/Replayが差し替え可能
- [ ] 固定dt + seed固定 + Replay再現性
- [ ] Domain変更にテストが付いている

---
