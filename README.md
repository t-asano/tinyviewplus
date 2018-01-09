# Tiny View Plus

Tiny View Plusは、FPV映像を3画面同時に表示できるアプリです。UVC対応のFPV受信機を、MacBook等のコンピューターに接続して利用します。

![tinyviewplus](https://user-images.githubusercontent.com/14119184/34721592-a1043ef2-f586-11e7-9e4a-c73a2d42d59e.jpg)

## 動作環境

- MacBook Pro + MacOS High Sierra
- Eachine ROTG01 / ROTG02

### 補足

- ソースコードからビルドする場合は openFrameworks v0.9.8 が必要です
- Windowsでの動作は未検証です

## インストール

### ビルド済みバイナリを使用する場合(MacOSのみ)

1. [リリースページ](https://github.com/t-asano/tinyviewplus/releases)よりzipファイルをダウンロード
2. zipファイルを適当な場所に展開

### ソースコードからビルドする場合

1. [openFrameworks](http://openframeworks.cc/ja/) v0.9.8 をセットアップ
2. 新規プロジェクトの雛形を作成
3. このリポジトリのソースコード(src/)を組み込み
4. ビルド

## 起動

1. コンピューターにFPV受信機を接続
2. アプリを起動

### 補足

- アプリ起動時に接続済みの受信機のみが使用されます
- 複数の受信機がうまく検出されない場合は、1台ずつ時間をおいて接続して下さい

## ライセンス

MIT License
