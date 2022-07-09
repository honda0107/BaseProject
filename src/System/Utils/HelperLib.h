#pragma once
#include <string>
#include <filesystem>
#include <cassert>

namespace HelperLib::File
{

using Files_t = std::vector<std::string>;

//! @brief FilePathからディレクトリ部分を取得する
//! @param filepath FilePath
//! @return ディレクトリ部分
std::string GetDirectory(std::string_view filepath);

//! @brief FilePathからエクステンション部分を取得する
//! @param filepath FilePath
//! @return エクステンション部分
std::string GetFileExtension(std::string_view filepath);

//! @brief FilePathからファイル名部分を取得する
//! @param filepath FilePath
//! @return ファイル名部分
std::string GetOnlyFileName(std::string_view filepath);

//! @brief ディレクトリパスのファイルを列挙します
//! @param dirname ディレクトリパス
//! @param ext 列挙する拡張子(ディフォルトは無視) 例:".png" ".jpg/.png/.bmp" など
//! @return ディレクトリに存在するファイル
Files_t GetFilesFromDirectory(std::string_view dirname, std::string_view ext = "");

//! @brief ファイルの存在チェック
//! @param path ファイルパス
//! @retval true  存在する
//! @retval false 存在しない
bool CheckFileExistence(std::string_view path);

//! @brief フォルダの作成
//! @param path パス (途中も作成する)
//! @retval true 作成した場合
//! @retval false 作成しなかった場合(すでに存在)
bool CreateFolder(std::string_view path);

}   // namespace HelperLib::File

namespace HelperLib::String
{

//! @brief @brief stringからwstringに変換します
//! @param string string文字列
//! @return string文字列
std::wstring ToWString(std::string_view string);

//! @brief wstringからstringに変換します
//! @param wstring wstring文字列
//! @return string文字列
std::string ToString(std::wstring_view wstring);

//! @brief wstringをu16stringに変換します
//! @param wstring wstring文字列
//! @return u16string文字列
std::u16string To16Str(std::wstring_view wstring);

//! @brief stringをあるセパレート文字で分解します
//! @param string 分解したい文字列
//! @param separator 区切り文字
//! @return 分解した文字列のリスト
std::vector<std::string> Split(std::string_view string, char separator);

}   // namespace HelperLib::String

namespace HelperLib::OS
{
//! @brief Windows11かをチェックします
//! @retval true : Windows11
//! @retval false: not Windows11
bool IsWindows11();

//! @brief Windows10以上かをチェックします
//! @retval true : Windows10以上
//! @retval false: Windows10以前のOS
bool IsWindows10OrGreater();

}   // namespace HelperLib::OS

namespace HelperLib::Math
{
bool NearlyEqual(float f1, float f2);

//! @brief ベクトルからMatrixを作成します
//! @param front 前ベクトル( Z-Vector )
//! @param up 上ベクトル (仮 Y-Vector )
//! @param upremake (上ベクトルを作り直すか?)
//! @return マトリクス
matrix CreateMatrixByFrontVector(float3 front, float3 up = float3{0, 1, 0}, bool upremake = true);

//! @brief 自分からターゲットを見るマトリクスの作成
//! @param my_object_pos 自分の位置
//! @param target_object_pos ターゲットの位置
//! @return ターゲットのほうに向いているマトリクス
matrix LookAtMatrixForObject(float3 my_object_pos, float3 target_object_pos);

}   // namespace HelperLib::Math
