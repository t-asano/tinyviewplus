# Tiny View Plus

Tiny View Plusは、FPV映像を手軽に表示するためのアプリです。UVC対応の受信機を、MacBook等のコンピューターに接続して使用します。openFrameworksを使って実装されており、高速に動作します。

![tinyviewplus](docs/overview.jpg)

## 動作環境

<table>
<tr>
<td>OS</td><td>macOS (High Sierra推奨)</td>
</tr>
<tr>
<td>受信機</td><td>Eachine ROTG02, ROTG01</td>
</tr>
</table>

ソースコードからビルドする場合には [openFrameworks](http://openframeworks.cc/ja/) v0.9.8 が必要です

## インストール

### ビルド済みバイナリを使用する場合

1. [リリースページ](https://github.com/t-asano/tinyviewplus/releases)よりzipファイルをダウンロード
2. zipファイルを適当な場所に展開

バイナリの作成にあたっては細心の注意を払っていますが、アプリを実行することで起こる不具合については、一切責任を負いません。

### ソースコードからビルドする場合

1. [openFrameworks](http://openframeworks.cc/ja/) v0.9.8 をインストール
- [CVPixelBufferに関する修正パッチ](https://github.com/openframeworks/openFrameworks/commit/836fbda74770b7a1df3e136e9d2200b5c2cee8a4)を適用(macOSのみ)
- ofxOscアドオンの[最新版](https://github.com/openframeworks/openFrameworks/tree/master/addons/ofxOsc)を addons/ofxOsc に上書き
- [ofxTrueTypeFontUCアドオン](https://github.com/hironishihara/ofxTrueTypeFontUC)を addons/ の下に配置
- プロジェクトジェネレーターで上記2つのアドオンを含むプロジェクトを作成
- ソースコード(src/)とアセット(bin/data/)を上書き
- ビルド

## 使い方

### 起動

1. コンピューターに受信機を接続
2. アプリ本体をダブルクリックして起動

macOSにおいて、ビルド済みバイナリを起動しようとすると、以下のような警告が表示されることがあります。

![binary01](docs/binary01.png)

この場合、[システム環境設定] -> [セキュリティとプライバシー] -> [一般]を開き [このまま開く] をクリックすると、アプリを起動できます。

![binary02](docs/binary02.png)

### 受信機の検出

受信機がうまく検出されない場合は、以下をお試し下さい。

1. アプリを終了
2. 受信機を全て外す
3. USBハブ使用の場合はそれを接続し直す、または撤去する
4. 受信機を一台ずつ数秒間隔で接続
5. アプリを起動

macOSの場合、QuickTimeアプリで、カメラ(受信機)のリストを表示できます。Tiny View Plusを実行する前に、カメラの検出状況を確認しておくと、問題の切り分けがしやすくなります。

Windowsでは、デバイスマネージャーでカメラ(受信機)の検出状況を確認できます。なお、一つのUSBハブに複数の受信機を接続した場合に、そのうち1台しか動作しないことがあります。この場合、USBハブを経由せずに接続すれば、改善することがあります。

### キー操作

| キー | 機能 | 初期値 |
|---|---|---|
| H | ヘルプの表示 | - |
| 1~3 | カメラ1~3の表示/非表示 | 表示 |
| Q,W,E | カメラ1~3のアイコンの変更 | アプリ内蔵の画像(data/default\_pilot\_icon.png) |
| L | カメラのラベルの変更 | Pilot1~3 |
| B | 背景画像の変更 | アプリ内蔵の画像(data/default\_backgroud.png) |
| S | 音声読み上げのオン/オフ(macOSのみ) | オフ |
| R | 設定の初期化(※1) | 各設定の初期値 |

- カメラのラベルを変更すると、アイコンが自動的に変更されます
	- macOSバイナリの場合は "Tiny View Plus.app/Contents/Resources/data/pilots" フォルダの下、それ以外の場合は "data/pilots" フォルダの下に、ラベル文字列.png または ラベル文字列.jpg という画像ファイルが見つかると、この優先順でアイコン画像として採用されます
	- 画像ファイルが見つからない場合は、デフォルトアイコンが採用されます
	- 縦横比は強制的に1:1となります
- 背景画像は、画面に合わせて拡大縮小され、左上が優先表示されます
- 音声読み上げは、外部アプリとの連携時に動作します
- (※1) 以下の設定項目が初期化されます
	- カメラの表示/非表示
	- カメラのアイコン
	- カメラのラベル
	- 音声読み上げのオン/オフ
	- 壁紙
- アプリを終了すると設定は消去されます
- カメラの音声は出力されません

## OSCによる制御

OSCプロトコルにより外部からの制御が可能です。UDP4000番ポートでOSCパケットを受信します。

### メッセージフォーマット

#### カメラの表示/非表示
/v1/camera/{id}/display {switch}

- パラメーター
    - id ... 1～3の数値
    - switch ... "on" または "off"
- [例]カメラ1を非表示とする
    - /v1/camera/1/display "off"

#### カメラのラベルの変更
/v1/camera/{id}/label {label}

- パラメーター
    - id ... 1～3の数値
    - label ... 任意の文字列(パイロット名など)
- [例]カメラ2のラベルを"Whooper2"とする
    - /v1/camera/2/label "Whooper2"

ラベルの変更と連動して、アイコンも自動的に変更されます。

- macOSバイナリの場合は "Tiny View Plus.app/Contents/Resources/data/pilots" フォルダの下、それ以外の場合は "data/pilots" フォルダの下に、{label}.png または {label}.jpg という画像ファイルが見つかると、この優先順でアイコン画像として採用します
- 画像ファイルが見つからない場合は、デフォルトアイコンが採用されます
- 縦横比は強制的に1:1となります

#### カメラのラップタイムの更新
/v1/camera/{id}/laptime {time}

- パラメーター
    - id ... 1～3の数値
    - label ... ラップタイムを示す数値(秒)
- [例]カメラ3のラップタイムを128秒64とする
    - /v1/camera/3/laptime 128.64
- [例]カメラ3のラップタイムをクリアする
    - /v1/camera/3/laptime 0

#### 音声読み上げ(macOSのみ)
/v1/speech/{language}/say {text}

- パラメーター
    - language ... "en"(英語)または"jp"(日本語)
    - text ... 読み上げる文
- [例]日本語で"ドローンで遊ぼう"と読み上げる
    - /v1/speeech/jp/say "ドローンで遊ぼう"

### 動作確認

以下のようにして、Node.jsで動作を確認できます。

```js
var osc = require('node-osc');
var client = new osc.Client('127.0.0.1', 4000);
client.send("/v1/camera/1/label", "Whooper1", function () {});
client.send("/v1/camera/1/laptime", 62.09, function () {});
```

## License

Tiny View Plus is distributed under the MIT License. This gives everyone the freedoms to use Tiny View Plus in any context: commercial or non-commercial, public or private, open or closed source. Please see [LICENSE](LICENSE) for details.