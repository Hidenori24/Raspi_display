# Golf Simulator - Simple Prototype

C++ + raylib で作ったシンプルなプロトタイプです。  
グリーン上にボール位置と距離データを表示します。

## ビルド方法

### 必要なもの
- CMake 3.10以上
- C++17 対応コンパイラ
- raylib 5.0（自動ダウンロード）

### macOS / Linux
```bash
cd golf-sim
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

### 実行
```bash
./golf-sim
```

## 機能（MVP）
- ✅ グリーン表示（2D俯瞰図）
- ✅ ボール位置表示（カラフルなボール）
- ✅ 距離データ表示（HUD）
- ✅ グリッドと距離表示
- ✅ FPS表示

## 今後の拡張
1. イベントキューの実装
2. 物理エンジンの統合
3. 軌跡の描画
4. ゲーム状態遷移（FSM）
5. 実センサー入力対応
