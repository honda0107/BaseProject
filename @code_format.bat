@echo off
@setlocal enabledelayedexpansion

pushd "%~dp0"

set CLANG_FORMAT_CONFIG=.clang-format-reference

echo 全ソースコードを整形します

:: ユーザー定義ファイルで変換
if "%1"=="User" (
    echo ユーザー定義の整形を実行します
    set CLANG_FORMAT_CONFIG=.clang-format
)

::---------------------------------------------------------------
:: VisualStudio内のclag-formatのパスを検出
::---------------------------------------------------------------
set VSWHERE_PATH="C:/Program Files (x86)/Microsoft Visual Studio/Installer/vswhere.exe"

if not exist %VSWHERE_PATH% (
	echo VisualStudioがインストールされていません.
	goto error_finish
)

:: VisualStudioがインストールされているパスを取得
for /f "usebackq delims=" %%i in (`%VSWHERE_PATH% -latest -property installationPath`) do (
	set VS_PATH=%%i
)

:: VisualStudio内のLLVMのパス
set VS_LLVM_PATH=%VS_PATH%\VC\Tools\Llvm\x64\bin

:: clang-formatのパス
set CLANG_FORMAT_PATH="%VS_LLVM_PATH%\clang-format.exe"

::---------------------------------------------------------------
:: clang-formatが実行できるかどうかチェック
::---------------------------------------------------------------
if not exist %CLANG_FORMAT_PATH% (
	echo clang-formatが見つかりません.
	goto error_finish
)

:: ローカルのclang-formatのフルパスを利用 (styleにファイル指定できる機能がLLVM14からのみのため)
:: VisutalStudioがLLVM14ベースになったらこの処理は不要になります
for /f "delims=" %%f in ('dir /b /s  bin\clang-format.exe') do (
  set CLANG_FORMAT_PATH="%%f"
)

:: バージョン表示
%CLANG_FORMAT_PATH% --version

::---------------------------------------------------------------
:: src直下のすべてのソースコードにclang-formatを実行
::---------------------------------------------------------------
pushd "%~dp0src"

for /f "delims=" %%f in ('dir /b /s src *.c *.cpp *.cxx *.cc *.hpp *.h *.hxx *.inl') do (
  set /p X="." < nul
  %CLANG_FORMAT_PATH% -i -style=file:%CLANG_FORMAT_CONFIG% "%%f"
)

echo 完了しました.

goto finish

:error_finish
popd
pause

exit /b 1

:finish
popd

exit /b 0
