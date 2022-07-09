# BaseProject2022
## 概要
### BaseProjectとは？
DXライブラリを使用して3Dゲーム開発をするための最もシンプルなプロジェクトです。<br />

## 使用環境
- OS: Windows10, Windows11
- IDE: VisualStudio2022

## 使い方
### プロジェクトの起動方法:

「＠open.bat」ファイルをダブルクリックしてください。<br />
自動的にVisualStudio2022でプロジェクトが開きます。<br />
<b>注意事項: 2バイト文字が含まれているドライブまたはフォルダ上では正常にバッチが動作しません</b>

### プロジェクトの終了方法

「＠cleanup.bat」をダブルクリックしてください。<br />
自動的に不要なファイルが削除されます。<br />

### コードの評価

「＠code_metrics.bat」をダブルクリックしてください。<br />
自動的に「src」フォルダ内のソースファイル全てが「cccc」によって評価されます。<br />

### Doxygenの作成／修正

「＠doxygen.bat」をダブルクリックしてください。<br />
自動的に「src」フォルダ内のソースファイル全てを解析してドキュメントを作成します。<br />
<b>注意事項: Doxygen形式でのコメントの書き方をしていないとドキュメントに反映されません</b>

### コード整形

「src/.clang-format-reference」を「.clang-format」にリネームすることで自分好みのソースコードに整形が可能です。<br />
また「＠open.bat」を使用することで自動的にこのファイルを読み込み、コードを自分好みに整形してくれます。<br />
<b>注意事項: 「＠code_format.bat」を使うと元のコード整形に戻ります</b>

## ライセンス
著作権保有者はBaseProject2022の著作権を放棄していません。<br />
無料ソフト、有料ソフト問わず、BaseProject2022を使用して作成されたソフトウエアに対するライセンス料等は(商用利用・法人利用問わず)一切発生しません。<br />
BaseProject2022を用いたことによって生じた如何なる損害に対しても著作権保有者はその保障義務を一切負わないものとします。<br />
著作権保有者はBaseProject2022に不備が有っても、それを訂正する義務を負いません。<br />
BaseProject2022を使用して作成されたソフトウエアにBaseProject2022を使用した旨を記載する必要はありませんが、<br />
BaseProject2022を使用して作成されたソフトウエアに同梱されているライブラリやデータ等の著作権や使用明記の必須に合わせてください。

## 最後に
BaseProjectはみんなのものです。<br />
誰かがひとりじめしないよう、BaseProjectを利用して得られた知識は周りに還元してください。<br />
