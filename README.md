# Tiny View Plus

Tiny View Plusは、FPV映像を3画面同時に表示可能なアプリです。UVC対応のFPV受信機を、MacBook等のコンピューターに接続して使用します。openFrameworksを使って実装されており、Webアプリとして実装されていた前作([TinyView](https://github.com/t-asano/tinyviewplus/))よりも描画性能が向上しています。

![tinyviewplus](https://user-images.githubusercontent.com/14119184/34721592-a1043ef2-f586-11e7-9e4a-c73a2d42d59e.jpg)

## 動作環境

- macOS High Sierra
	- ソースコードからビルドする場合には [openFrameworks](http://openframeworks.cc/ja/) v0.9.8 が必要
	- macOS以外でも動作する可能性はあるが未検証
- Eachine ROTG01 / ROTG02

## インストール

### ビルド済みバイナリを使用する場合(macOSのみ)

1. [リリースページ](https://github.com/t-asano/tinyviewplus/releases)よりzipファイルをダウンロード
2. zipファイルを適当な場所に展開

### ソースコードからビルドする場合

1. [openFrameworks](http://openframeworks.cc/ja/) v0.9.8 をインストール
2. [CVPixelBufferに関する修正パッチ](https://github.com/openframeworks/openFrameworks/commit/836fbda74770b7a1df3e136e9d2200b5c2cee8a4)を適用
3. 新規プロジェクトを作成
4. [ofxTrueTypeFontUCアドオン](https://github.com/hironishihara/ofxTrueTypeFontUC)を追加
5. ソースコード(src/)とアセット(bin/data/)を取り込み
6. ビルド

## 起動

1. コンピューターにFPV受信機を接続
2. アプリを起動
	- アプリ起動時に接続されている受信機のみを使用します
	- 複数の受信機がうまく検出されない場合は、1台ずつ時間をおいて接続して下さい

## License

Tiny View Plus is distributed under the MIT License. This gives everyone the freedoms to use Tiny View Plus in any context: commercial or non-commercial, public or private, open or closed source. Please see [LICENSE](LICENSE) for details.
