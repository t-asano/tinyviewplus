# Tiny View Plusのビルド

## 開発環境のセットアップ

### macOSの場合

1. Xcodeをインストールする
   - [App Store](https://itunes.apple.com/ca/app/xcode/id497799835?mt=12)からXcodeをインストールする÷
2. openFrameworks 0.12.0をインストールする
   - [openFrameworks for osx (zipファイル)](https://openframeworks.cc/download/)をダウンロードする
   - 拡張属性を取り除く
     - $ xattr -cr of\_\*\_osx_release.zip
   - 適当な場所に展開する

### Windowsの場合

1. Visual Studioをインストールする
   - [こちらのドキュメント](https://openframeworks.cc/setup/vs/)を参考とする
   - 「openFrameworks plugin for Visual Studio」は実施しない
2. openFrameworks 0.11.2をインストールする
   - [openFrameworks for visual studio (zipファイル)](https://openframeworks.cc/download/)をダウンロードする
   - 適当な場所に展開する
3. [Git for Windows](https://gitforwindows.org)をインストールする
   - これに含まれるGit Bashを使って、GitHubからのファイルの取得や、ローカルでのファイル操作を行うことになる

## プロジェクトの作成

1. 必要なアドオンをインストールしておく
   
   ```bash
   $ cd addons/
   $ git clone -b fix-of_v0.10 https://github.com/t-asano/ofxTrueTypeFontUC.git
   $ git clone https://github.com/t-asano/ofxAruco.git
   $ git clone https://github.com/kylemcdonald/ofxCv.git
   $ git clone https://github.com/t-asano/ofxZxing.git
   $ git clone https://github.com/t-asano/ofxJoystick.git
   ```

2. プロジェクトジェネレーターで、以下のアドオンを含むプロジェクトを作成する。プロジェクト名は「tinyviewplus」とする。
   
   - ofxOsc
   - ofxTrueTypeFontUC
   - ofxAruco
   - ofxCv
   - ofxOpenCv
   - ofxPoco
   - ofxZxing
   - ofxJoystick
   - ofxXmlSettings

3. 作成したプロジェクトに、Tiny View Plusのファイル一式をマージする
   
   ```bash
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

2. ソースファイルの文字コードを固定する
   
   - tinyviewplusプロジェクトのプロパティを開く
   - [構成プロパティ] -> [C/C++] -> [コマンドライン]  と進む
   - 「全ての構成」「すべてのプラットフォーム」に対して、「追加のオプション」として「/source-charset:utf-8」を追加する

3. アプリ起動時にコンソールが表示されるのを避ける
   
   - tinyviewplusプロジェクトのプロパティを開く
   - [構成プロパティ] -> [リンカ] -> [システム] と進む
   - 「全ての構成」「すべてのプラットフォーム」に対して、「サブシステム」として「Windows (/SUBSYSTEM:WINDOWS)」を指定する
   - [構成プロパティ] -> [リンカ] -> [詳細設定] と進む
   - 「全ての構成」「すべてのプラットフォーム」に対して、「エントリポイント」として「mainCRTStartup」を指定する

4. 文字化けを防ぐ
   
   - openFrameworks の ofSystemUtils.cpp を改造する
     
     ```cpp
     static void narrow(const std::wstring &src, std::string &dest) {
         setlocale(LC_CTYPE, "");
         char *mbs = new char[src.length() * MB_CUR_MAX + 1];
         wcstombs(mbs, src.c_str(), src.length() * MB_CUR_MAX + 1);
         dest = mbs;
         delete[] mbs;
     }
     
     std::string convertWideToNarrow( const wchar_t *s, char dfault = '?', const std::locale& loc = std::locale() )
     {
     #if 0
         std::ostringstream stm;
         while (*s != L'\0') {
            stm << std::use_facet< std::ctype<wchar_t> >(loc).narrow(*s++, dfault);
         }
         return stm.str();
     #else
         std::wstring wstr(s);
         std::string ret;
         narrow(wstr, ret);
         return ret;
     #endif
     }
     ```

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
   
   ```bash
   $ cd apps/myApps/tinyviewplus
   $ git pull
   $ cd ../../../addons/ADDON_NAME
   $ git pull
   ```

2. アドオンを追加する(必要な場合のみ)
   
   - Gitコマンドで新規のアドオンを取得する
   - プロジェクトジェネレーターでアドオンを追加する

以上