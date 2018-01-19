# Tiny View Plus

Tiny View Plusは、FPV映像を3画面同時に表示可能なアプリです。UVC対応の受信機を、MacBook等のコンピューターに接続して使用します。

openFrameworksを使って実装されており、Webアプリとして実装されていた前作([Tiny View](https://github.com/t-asano/tinyview))よりも描画性能が向上しています。

![tinyviewplus](docs/overview.jpg)

## 動作環境

<table>
<tr>
<td>OS</td><td>macOS (High Sierra以降を推奨)</td>
</tr>
<tr>
<td>受信機</td><td>Eachine ROTG02, ROTG01</td>
</tr>
</table>

- ソースコードからビルドする場合には [openFrameworks](http://openframeworks.cc/ja/) v0.9.8 が必要です
- Windows環境でもビルドできる可能性がありますが、未検証です

## インストール

### ビルド済みバイナリを使用する場合

1. [リリースページ](https://github.com/t-asano/tinyviewplus/releases)よりzipファイルをダウンロード
2. zipファイルを適当な場所に展開

### ソースコードからビルドする場合

1. [openFrameworks](http://openframeworks.cc/ja/) v0.9.8 をインストール
2. [CVPixelBufferに関する修正パッチ](https://github.com/openframeworks/openFrameworks/commit/836fbda74770b7a1df3e136e9d2200b5c2cee8a4)を適用
3. 新規プロジェクトを作成
4. [ofxTrueTypeFontUCアドオン](https://github.com/hironishihara/ofxTrueTypeFontUC)を追加
5. ソースコード(src/)とアセット(bin/data/)を取り込み
6. ビルド

## 使い方

### 起動

1. コンピューターに受信機を接続
2. アプリを起動

受信機がうまく検出されない場合は、以下をお試し下さい。

1. アプリを終了
2. 受信機を全て外す
3. 受信機を一台ずつ数秒間隔で接続
4. アプリを起動

### キー操作

| キー | 動作 | 初期値 |
|---|---|---|
| 1~3 | カメラ1~3の表示/非表示 | 表示 |
| H, h | ヘルプの表示 | - |
| L, l | カメラのラベルの変更 | CAM1~3 |
| R, r | 設定の初期化(※1) | 各設定の初期値 |
| W, w | 壁紙の変更 | アプリ内蔵の壁紙 |

- (※1)カメラの表示/非表示、ラベル、壁紙が初期化されます
- 壁紙は、縦横比を維持しながらウィンドウサイズに合わせて拡大縮小され、左上が優先的に表示されます
- アプリを終了すると設定は消去されます

## License

Tiny View Plus is distributed under the MIT License. This gives everyone the freedoms to use Tiny View Plus in any context: commercial or non-commercial, public or private, open or closed source. Please see [LICENSE](LICENSE) for details.