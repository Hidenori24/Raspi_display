# AGENTS.md — Copilot Coding Agent Operating Rules
Raspberry Pi 5 Golf Simulator (C++ / DDD / Clean Architecture)

このファイルは、GitHub Copilot Coding Agent が Issue を実装して PR を作る際に守るべき「行動規範」「作業手順」「品質ゲート」を定義します。  
リポジトリ内の `.github/copilot-instructions.md`（全体理念）と矛盾しない範囲で、**より手順的・運用的**な規約をここに記載します。

---

## 0. 目的（Agentのミッション）
- DDD / Clean Architecture を維持しつつ、Issue を確実に完了させる。
- 変更の影響範囲を最小化し、回帰（Replay）と単体テストで品質を担保する。
- 「動くが汚い」「動かないが綺麗」のどちらにも寄らず、**構造と実用性を両立**する。

---

## 1. 最優先ルール（Non-negotiable）
1. **Domain層は純粋**：I/O、raylib、スレッド、sleep、現在時刻、非決定乱数を持ち込まない。
2. **依存方向は外→内のみ**：Presentation/Infrastructure → Application → Domain を破らない。
3. **差し替え可能性を守る**：Real/Mock/Replay が interface と factory/config で切替可能であること。
4. **決定論を守る**：物理は固定dt、Mockはseed固定、Replayは再現性。
5. **PRにはテストが伴う**：Domainロジック追加・変更には単体テストを付ける（例外は明記）。

---

## 2. 作業開始時の手順（Issueを受け取ったら最初にやること）
1. Issue本文・添付・既存コメントを読み、要件を **「受入条件（Acceptance Criteria）」**に分解する。
2. 変更対象をレイヤごとに分類する（Domain / Application / Infrastructure / Presentation）。
3. **設計案を先に書く**（PR description か最初のコミットメッセージに含めてよい）：
   - 追加/変更するクラス
   - 依存関係（どの層に置くか）
   - テスト方針（単体/Replay/両方）
4. 不明点がある場合：
   - 可能な限り仮定を置いて進める（「仮定A」「仮定B」）。
   - 重大な分岐（挙動が変わりうる）だけ PR コメントに質問として残す。
   - “止まって質問待ち” にならないようにする。

---

## 3. 変更設計の原則（Agentが採用すべきデザイン）
### 3.1 追加は「内側を守って外側に拡張」
- Domainのモデル／不変条件を先に定義し、Applicationでユースケースとして束ねる。
- Infrastructureは “実装” を提供し、Presentationは “表示” を提供するだけ。

### 3.2 イベントとコマンドを混ぜない
- Input（センサー）→ InputEvent
- Application → Command（意図）
- Domain → DomainEvent（事実）

### 3.3 既存構造の尊重
- 既存のインタフェースや用語（Ubiquitous Language）を優先する。
- 似た責務のクラスを増やさず、まず既存クラスの責務範囲を確認する。

---

## 4. ファイル配置のルール（迷ったらここ）
> ※プロジェクトのフォルダ命名が `domain/ application/ infrastructure/ presentation/` でない場合も、**責務**は必ず合わせること。

- **Domain**：Entity / Value Object / Domain Service / Domain Event
- **Application**：UseCase / CommandHandler / Orchestrator
- **Infrastructure**：SensorProvider（Real/Mock/Replay）/ Config / Logger / Trace I/O
- **Presentation**：Renderer / HUD / UI入力 / raylib依存コード

禁止：
- Domainに `raylib.h` や CSV/JSON パーサや OS API を置かない。
- Presentationが物理計算や検出ロジックを実装しない（読むだけ）。

---

## 5. 実装時のチェックポイント（C++具体ルール）
### 5.1 所有権
- 原則 `std::unique_ptr`
- `std::shared_ptr` は共有所有が本当に必要な場合のみ（理由をPRに書く）
- 引数は `const&` を基本、コピーは明示

### 5.2 例外・エラー
- Domainは原則例外で制御しない（戻り値/Expected等）
- I/O例外はInfrastructureで捕捉し、Applicationへ“制御された失敗”として返す

### 5.3 include
- 前方宣言を優先し循環参照を避ける
- raylib include は Presentation 限定
- JSON/CSV include は Infrastructure 限定

### 5.4 決定論
- 物理は固定dt
- Mockはseedを引数で受け取れる
- Replayはトレース同一なら結果が一致（許容誤差を定義）

---

## 6. テスト戦略（Agentが必ずやる）
### 6.1 単体テスト（Domain中心）
- Physics/ballistics/ルールは単体テストを書く
- 同じ入力で同じ出力（seed固定、固定dt）

### 6.2 結合テスト（Replay）
- Replay trace → ShotResult がスナップショット一致（または許容誤差内）
- 可能なら `--headless` 実行で CI に載せられる形にする

### 6.3 テストが難しい場合
- なぜ難しいかをPRで明記
- 代替（簡易テスト、Replay追加、境界をインタフェース化）を必ず提示

---

## 7. コミット/PR作法（Agentのアウトプット規約）
### 7.1 コミット粒度
- “設計骨格（interface/DTO）” → “実装” → “テスト” → “ドキュメント” の順で分けるのが理想
- 1コミットで大量の無関係変更を混ぜない

### 7.2 PR本文に必ず書くこと
- 目的 / 変更概要
- 影響範囲（どの層を触ったか）
- 設計判断（なぜそのパターン/配置にしたか）
- テスト方法（実行コマンド、Replayの再生方法、期待結果）
- 既知の制限（MVPの仮定、未対応）

### 7.3 変更規模が大きい場合
- 先に “小さく縦に通す” PR を優先（例：Mock→Physics→HUDまで）
- その後、検出精度やUI強化などを分割PRで進める

---

## 8. 受入条件（Definition of Done）
Issueが完了したと判断する最低条件：

- [ ] 依存方向（外→内）が守られている
- [ ] Domainが純粋である（I/O/描画/スレッド無し）
- [ ] Real/Mock/Replay が差し替え可能（config/factoryで切替）
- [ ] 物理は固定dtで、Mockはseed固定、Replayは再現可能
- [ ] 追加/変更されたDomainロジックにテストがある
- [ ] 主要ユースケースが手順通り実行できる（PRに手順記載）
- [ ] 不明点や仮定は PR に明記されている

---

## 9. “よくある崩壊パターン” の禁止（Agent向け）
- “Rendererで物理計算” を始める（プレゼン層が肥大化）
- “Configをどこでも読めるシングルトン” を作る（密結合化）
- “とりあえず便利だからDomainでログ” を始める（純粋性崩壊）
- “RealSensor導入が先” で Mock/Replay が後回し（回帰不能になる）

---

## 10. 推奨ワークフロー（最短で品質を担保する）
1. Domain：モデルと不変条件、計算（テスト付き）
2. Application：ユースケース（Command→Domain）
3. Infrastructure：Mock/Replayでデータ供給（seed/trace）
4. Presentation：raylibで状態を表示（読むだけ）
5. Replay回帰を通して、以後の変更で壊れない基盤を作る

---

## 11. Agentが生成すべき最小成果物（例）
Issueが「機能追加」であれば、可能な範囲で以下をセットで出す：

- 新規/変更クラス（レイヤ遵守）
- 単体テスト（Domain）
- Replay用トレース（必要なら最小1本）
- 実行手順（CLI例）
- 変更理由（PR本文）

---

この AGENTS.md は、Copilot Coding Agent が自律的に実装を進める際の “運用規約” です。  
迷ったら「Domainを純粋に」「依存は外→内」「Replayで再現性」を優先してください。