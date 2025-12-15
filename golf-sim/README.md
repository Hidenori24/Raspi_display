# Golf Simulator - Refactored Architecture

C++ + raylib で作ったゴルフシミュレータのプロトタイプです。  
**Clean Architecture / DDD 原則に基づいて構造化されています。**

## 📐 Architecture

このプロジェクトは以下の4層で構成されています：
- **Domain Layer** (Pure C++): 物理エンジン、ゲーム状態機械（決定論的）
- **Application Layer**: ユースケース、アプリケーションサービス
- **Infrastructure Layer**: センサープロバイダ（Mock/Replay/Real）
- **Presentation Layer**: レンダラー、UI（raylib依存）

詳細は [ARCHITECTURE.md](ARCHITECTURE.md) を参照してください。

## ビルド方法

### 必要なもの
- CMake 3.10以上
- C++17 対応コンパイラ
- raylib 5.0（自動ダウンロード）
- X11開発ライブラリ（Linux）
  ```bash
  sudo apt-get install libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev libgl1-mesa-dev
  ```

### ビルド手順
```bash
cd golf-sim
mkdir build && cd build
cmake ..
make -j4
```

### 実行
```bash
./golf-sim
```

### テスト実行
```bash
./tests/test_physics  # ドメイン層の単体テスト
```

## 操作方法
- **LEFT/RIGHT**: クラブ選択
- **UP/DOWN**: パワー調整
- **A/D**: 狙い調整
- **SPACE**: ショット実行 / 次のホールへ

## 機能（実装済み）
- ✅ レイヤ分離アーキテクチャ（Domain/App/Infra/Presentation）
- ✅ 固定タイムステップ物理エンジン（決定論的）
- ✅ ゲーム状態機械（Idle/Armed/InFlight/Result）
- ✅ 弾道計算（重力 + 抗力 + 風）
- ✅ 軌跡表示（2D俯瞰図）
- ✅ モックセンサープロバイダ（シード固定で再現可能）
- ✅ ドメイン層単体テスト（全テスト合格）
- ✅ CMakeによる境界強制（依存方向の保証）

## 今後の拡張
- [ ] ReplaySensorProvider（CSV/JSONトレース再生）
- [ ] EventQueue（ドロップメトリクス付き）
- [ ] JSON設定ファイル読み込み
- [ ] イベント/結果のCSV/JSONログ出力
- [ ] CLIオプション（--sensor, --trace, --headless）
- [ ] Magnusフォース（スピン効果）
- [ ] 統合テスト（リプレイ検証）

## 設計原則
このコードベースは以下の原則に従っています：
1. **Domain純粋性**: I/O、raylib、時刻、乱数なし
2. **依存方向**: 外→内のみ（Presentation/Infra → App → Domain）
3. **差し替え可能性**: Interface + Factory で Mock/Replay/Real 切替
4. **決定論**: 固定dt、seed固定、同一入力→同一結果
5. **テスト必須**: ドメインロジック変更には単体テスト付与

詳細は [.github/copilot-instructions.md](../.github/copilot-instructions.md) を参照してください。
