## OSCによるTiny View Plusの制御

Tiny View Plusは、OSCプロトコルにより外部から制御可能です。UDP4000番ポートでOSCパケットを受信します。

## メッセージフォーマット

### カメラの拡大表示

/v1/camera/{id}/solo {switch}

- パラメーター
  - id ... カメラID(1～4)
  - switch ... "on" または "off"
- 例: カメラ1を拡大表示する
  - /v1/camera/1/solo "on"

### カメラの表示/非表示

/v1/camera/{id}/display {switch}

- パラメーター
  - id ... カメラID(1～4)
  - switch ... "on" または "off"
- 例: カメラ1を非表示とする
  - /v1/camera/1/display "off"

### カメラのラベルの変更

/v1/camera/{id}/label {label}

- パラメーター
  - id ... カメラID(1～4)
  - label ... 任意の文字列(パイロット名など)
- 例: カメラ2のラベルを"Whooper2"とする
  - /v1/camera/2/label "Whooper2"

ラベルの変更と連動して、アイコンも自動的に変更されます。

- macOSバイナリの場合は "Tiny View Plus.app/Contents/Resources/data/pilots" フォルダの下、それ以外の場合は "data/pilots" フォルダの下に、{label}.png または {label}.jpg という画像ファイルが見つかると、この優先順でアイコン画像として採用します
- 画像ファイルが見つからない場合は、デフォルトアイコンが採用されます
- 縦横比は強制的に1:1となります

### ラップタイムの設定

/v1/camera/{id}/laptime {time}

- パラメーター
  - id ... カメラID(1～4)
  - time ... ラップタイム(秒)
- 例: カメラ3のラップタイムを128秒64とする
  - /v1/camera/3/laptime 128.64

### ラップの追加と削除

/v1/camera/{id}/lap {command}

- パラメーター
  - id ... カメラID(1～4)
  - command ... "add"(追加)または"del"(削除)
- 例: カメラ3のラップを追加する
  - /v1/camera/3/lap "add"

### 音声読み上げ

/v1/speech/{language}/say {text}

- パラメーター
  - language ... "en"(英語)または"jp"(日本語)
  - text ... 読み上げ文
- 例: 日本語で"ドローンで遊ぼう"と読み上げる
  - /v1/speeech/jp/say "ドローンで遊ぼう"