@echo off
@setlocal enabledelayedexpansion

pushd "%~dp0"
call "@cleanup.bat" warm
popd

::---------------------------------------------------------------
:: Gitコミットフックを適用
::---------------------------------------------------------------
pushd "%~dp0"

if exist .git (
    copy bin\pre-commit .git\hooks > nul
)

popd

::---------------------------------------------------------------
:: clang-format定義ファイルをコピー
::---------------------------------------------------------------
pushd "%~dp0\src"

if not exist .clang-format (
    copy .clang-format-reference .clang-format > nul
    echo .clang-formatファイルを作成しました.
)

popd

::---------------------------------------------------------------
:: .buildフォルダを作成
:: 環境変数BUILD_PATHを設定した場合は作成先を移動可能。
::---------------------------------------------------------------
pushd "%~dp0"

:: すでにフォルダが存在していたら作成をスキップ
if exist .build goto skip

:: カスタムビルドパスの環境変数が存在しなかった場合はスキップ
if not defined BUILD_PATH goto skip

echo 環境変数BUILD_PATHが設定されています。%BUILD_PATH%
echo BUILD_PATH=%BUILD_PATH%

:: シンボリックリンク生成には管理者権限が必要なため自動昇格
whoami /priv | find "SeDebugPrivilege" > nul
if %errorlevel% neq 0 (
	echo 管理者権限がありませんでした。管理者権限に昇格して実行します
	powershell start-process "`%~0" -verb runas -wait
	goto skip
)

echo 管理者権限に自動で昇格できました。

:: テンポラリフォルダ用にGUIDを取得
for /f %%a in ('powershell -command "$([guid]::NewGuid().ToString())"') do ( set GUID=%%a )

:: シンボリックリンク接続先のフォルダ生成
if not exist "%BUILD_PATH%\.build\%GUID%" (
	md "%BUILD_PATH%\.build\%GUID%"
)

:: シンボリックリンク作成
mklink /d .build "%BUILD_PATH%\.build\%GUID%"

:：　終了
exit /b 0

:skip
::---------------------------------------------------------------
:: プロジェクトファイルを生成
::---------------------------------------------------------------

bin\premake5.exe vs2022

if %ERRORLEVEL% neq 0 (
	echo.
	echo エラーが発生しました。Luaスクリプトを確認してください。
	echo.
	echo ==========
	echo "～～/premake5.lua: No such file or directory" と赤い文字で表示される場合はフォルダパスに全角文字が含まれていないか
	echo 配置場所を確認してください。
	echo.
	echo [現在の配置場所]
	cd
	echo.
	echo 全角文字（日本語など）が含まれています。
	echo.
	echo ==========
	echo.
	pause
)

popd

::---------------------------------------------------------------
:: VisualStudio2022で並列ビルドにならない不具合の対策設定
:: (修正アップデートが入ったら不要になります)
::---------------------------------------------------------------
pushd "%~dp0"
copy "bin\Directory.Build.props" ".build" > nul
popd

:finish
exit /b 0
