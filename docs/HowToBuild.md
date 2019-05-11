# Tiny View Plusのビルド

## 開発環境のセットアップ

### macOSの場合

1. Xcodeをインストールする
   - [App Store](https://itunes.apple.com/ca/app/xcode/id497799835?mt=12)からXcodeをインストールする
   - Xcodeのcommand line toolsをインストールする
     - $ xcode-select --install
2. openFrameworks 0.10.1をインストールする
   - [zipファイル](https://openframeworks.cc/versions/v0.10.1/of_v0.10.1_osx_release.zip)をダウンロードする
   - 拡張属性を取り除く
     - $  xattr -cr of_v0.10.1_osx_release.zip
   - 適当な場所に展開する

### Windowsの場合

1. Visual StudioとopenFrameworksをインストールする
    - [こちらのドキュメント](https://openframeworks.cc/setup/vs/)を参考とする
      - Visual Studio Community 2017を前提に書かれているが、2019でも問題無い
      - 「openFrameworks plugin for Visual Studio」は実施しない
2. openFrameworks 0.10.1をインストールする
    - 適当な場所に[openFrameworks for visual studio](https://openframeworks.cc/download/)を展開する
3. Git for Windowsをインストールする
    - [こちらのドキュメント](https://qiita.com/elu_jaune/items/280b4773a3a66c7956fe)を参考にインストールする
    - これに含まれるGit Bashを使って、GitHubからのファイルの取得や、ローカルでのファイル操作を行うことになる

## プロジェクトの作成

1. 必要なアドオンをインストールしておく
    ```
    $ cd addons/
    $ git clone -b fix-of_v0.10 https://github.com/t-asano/ofxTrueTypeFontUC.git
    $ git clone https://github.com/t-asano/ofxAruco.git
    $ git clone https://github.com/kylemcdonald/ofxCv.git
    $ git clone https://github.com/t-asano/ofxZxing.git
    ```
	
2. プロジェクトジェネレーターで、以下のアドオンを含むプロジェクトを作成する。プロジェクト名は「tinyviewplus」とする。
    - ofxOsc
    - ofxTrueTypeFontUC
    - ofxAruco
    - ofxCv
    - ofxOpenCv
    - ofxPoco
    - ofxZxing
    
3. 作成したプロジェクトに、Tiny View Plusのファイル一式をマージする
    ```
    $ cd apps/myApps
    $ mv tinyviewplus tvptemp
    $ git clone https://github.com/t-asano/tinyviewplus.git
    $ cp tvptemp/* tinyviewplus/
    $ cp -r tvptemp/*.xcodeproj tinyviewplus/ # macOSの場合のみ
    $ rm -r tvptemp
    ```

## ビルドの設定

### macOSの場合

1. プロジェクトファイル(tinyviewplus.xcodeproj)をXcodeで開く

### Windowsの場合

1. ソリューションファイル(tinyviewplus.sln)をVisualStudioで開く

2. 文字コードを固定する
   - tinyviewplusプロジェクトのプロパティを開く
   - [構成プロパティ] -> [C/C++] -> [コマンドライン]  と進む
   - 「全ての構成」「すべてのプラットフォーム」に対して、「追加のオプション」として「/source-charset:utf-8」を追加する

3. オブジェクトファイルの競合を回避する
   - tinyviewplusプロジェクトのプロパティを開く
   - [構成プロパティ] -> [C/C++] -> [出力ファイル] と進む
   - 「オブジェクトファイル名」を「$(IntDir)/%(RelativeDir)/」に変更する

## ビルドと起動

### macOSの場合

1. ビルドターゲットを「tinyviewplus Release」に設定する
2. [Product] -> [Build] でビルドする
3. ビルドが成功したら [Product] -> [Run] で起動する

### Windowsの場合

1. ビルドターゲットを「Release」「x64」に設定する
2. [ビルド] -> [ソリューションのビルド] でビルドする
3. ビルドが成功したら [デバッグ] -> [デバッグの開始] で起動する

## アップデート

1. Tiny View Plusとアドオンを更新する
   ```
   $ cd apps/myApps/tinyviewplus
   $ git pull
   $ cd ../../../addons/ADDON_NAME
   $ git pull
   ```

2. アドオンの増減に追従する(必要な場合のみ)
   - Gitコマンドで新規のアドオンを取得する
   - プロジェクトジェネレーターでアドオンを取捨選択する