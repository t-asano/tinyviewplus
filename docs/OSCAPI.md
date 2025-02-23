# OSCによるTiny View Plusの制御と監視

Tiny View Plusは、OSCプロトコルによって、外部からの制御と監視が可能です。

## Tiny View Plusの制御(受信側)

UDP4000番ポートでOSCパケットを受信します。

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
  - /v1/camera/2/label "Whooper 2"

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

## Tiny View Plusの監視(送信側) ※実験的な機能

"settings.xml"で指定されたホスト・ポートへOSCパケットを送信します。

```xml
<oscMonitor>
    <enabled>1</enabled>
    <host>127.0.0.1</host>
    <port>4001</port>
</oscMonitor>
```

"settings.xml"は初回起動時に自動生成されます。この機能を有効にするには、Tiny View Plusを終了させた状態で、"enabled"に1を設定してください。無効にするには、0を設定してください。

### システム情報の通知

/v1/system/info {info}

- パラメーター
  - info ... システム情報
- 例: "hello"を通知する
  - /v1/system/info "hello"

### レースイベントの通知

/v1/race/event {event}

- パラメーター
  - event ... "started"または"finished"
- 例: レースの開始を通知する
  - /v1/race/event "started"

### カメラのラベルの通知

/v1/camera/{id}/label {label}

- パラメーター
  - id ... カメラID(1～4)
  - label ... カメラのラベル(パイロット名など)
- 例: カメラ2のラベルを通知する
  - /v1/camera/2/label "Whooper 2"

レース開始時およびラベル変更時に通知します。

### ラップの通知

/v1/camera/{id}/lap {lapnum} {laptime} {label}

- パラメーター
  - id ... カメラID(1～4)
  - lapnum ... ラップ番号
  - laptime ... ラップタイム(秒)
  - label ... カメラのラベル(パイロット名など)
- 例: カメラ3のラップを通知する(ラップ5、10.2秒)
  - /v1/camera/3/lap 5 10.2 "Whooper 3"

### ラップの削除の通知

/v1/camera/{id}/lapdel {lapnum}

- パラメーター
  - id ... カメラID(1～4)
  - lapnum ... ラップ番号
- 例: カメラ3のラップの削除を通知する(ラップ5)
  - /v1/camera/3/lapdel 5

### レース結果の通知

/v1/camera/{id}/result {label} {pos} {laps} {bestlap} {totaltime}

- パラメーター
  - id ... カメラID(1～4)
  - label ... カメラのラベル(パイロット名など)
  - pos ... 順位
  - laps ... ラップ数
  - bestlap ... ベストラップ(秒)
  - totaltime ... 総合タイム(秒)
- 例: カメラ1のレース結果を通知する(1位、10周、5.43秒、62.13秒)
  - /v1/camera/1/lap "Whooper 1" 1 10 5.43 62.13

レース終了時に通知します。
