//---------------------------------------------------------------------------
//! @file	HelperLib.cpp
//! @brief	ヘルパーライブラリ
//---------------------------------------------------------------------------
#include "HelperLib.h"
#include <VersionHelpers.h>

#include <string>
#include <sstream>   // std::stringstream
#include <istream>   // std::getline
#include <filesystem>
#include <cassert>
#include <memory>

namespace HelperLib::File
{
namespace fs = std::filesystem;

//! @brief FilePathからディレクトリ部分を取得する
//! @param filepath FilePath
//! @return ディレクトリ部分
std::string GetDirectory(std::string_view filepath)
{
    fs::path p{filepath};
    return p.parent_path().string();
}

//! @brief FilePathからエクステンション部分を取得する
//! @param filepath FilePath
//! @return エクステンション部分
std::string GetFileExtension(std::string_view filepath)
{
    fs::path p{filepath};
    return p.extension().string();
}

//! @brief FilePathからファイル名部分を取得する
//! @param filepath FilePath
//! @return ファイル名部分
std::string GetOnlyFileName(std::string_view filepath)
{
    fs::path p{filepath};
    return p.filename().string();
}

//! @brief ディレクトリパスのファイルを列挙します
//! @param dirname ディレクトリパス
//! @param ext 列挙する拡張子(ディフォルトは無視) 例:".png" ".jpg/.png/.bmp" など
//! @return ディレクトリに存在するファイル
Files_t GetFilesFromDirectory(std::string_view dirname, std::string_view ext)
{
    Files_t                          vfiles;
    fs::recursive_directory_iterator files(dirname);

    auto exts = String::Split(ext, '/');

    for(const auto& f : files) {
        auto path = f.path();
        if(ext == "") {
            vfiles.push_back(f.path().string());
            continue;
        }

        for(auto e : exts) {
            if(path.extension().string() == e) {
                vfiles.push_back(f.path().string());
                break;
            }
        }
    }

    return vfiles;
}

//! @brief ファイルの存在チェック
//! @param path ファイルパス
//! @retval true  存在する
//! @retval false 存在しない
bool CheckFileExistence(std::string_view path)
{
    return fs::exists(path);
}

//! @brief フォルダの作成
//! @param path パス (途中も作成する)
//! @retval true 作成した場合
//! @retval false 作成しなかった場合(すでに存在)
bool CreateFolder(std::string_view path)
{
    try {
        return fs::create_directories(path);
    }
    catch(std::filesystem::filesystem_error e) {
        assert(!"ホルダアクセスできません");
        return false;
    }
}
}   // namespace HelperLib::File

namespace HelperLib::String
{
//----------------------------------------------------------------------------
//! @brief @brief stringからwstringに変換します
//! @param string string文字列
//! @return string文字列
//----------------------------------------------------------------------------
std::wstring ToWString(std::string_view string)
{
    // 受け側の必要サイズを取得
    u32 size = MultiByteToWideChar(CP_ACP, 0, string.data(), static_cast<int>(string.length()), nullptr, 0);

    // バッファの取得
    std::wstring wstring;
    wstring.resize(size);

    // 変換
    MultiByteToWideChar(CP_ACP,
                        0,
                        string.data(),
                        static_cast<int>(string.length()),
                        wstring.data(),
                        static_cast<int>(wstring.size()));

    return wstring;
}

//----------------------------------------------------------------------------
//!	@brief wstringからstringに変換します
//!	@param wstring wstring文字列
//!	@return string文字列
//----------------------------------------------------------------------------
std::string ToString(std::wstring_view wstring)
{
    // 受け側の必要サイズを取得
    u32 size = WideCharToMultiByte(CP_ACP,
                                   0,
                                   wstring.data(),
                                   static_cast<int>(wstring.length()),
                                   nullptr,
                                   0,
                                   nullptr,
                                   nullptr);

    // バッファの取得
    std::string string;
    string.resize(size);

    // 変換
    WideCharToMultiByte(CP_ACP,
                        0,
                        wstring.data(),
                        static_cast<int>(wstring.length()),
                        string.data(),
                        static_cast<int>(string.size()),
                        nullptr,
                        nullptr);

    return string;
}

//----------------------------------------------------------------------------
//!	@brief wstringをu16stringに変換します
//!	@param wstring wstring文字列
//!	@return u16string文字列
//----------------------------------------------------------------------------
std::u16string To16Str(std::wstring_view wstring)
{
    // 処理系のバイトサイズが一致しているかチェック
    static_assert(sizeof(std::wstring::value_type) == sizeof(std::u16string::value_type), "size doesnt' match.");

    return std::u16string{wstring.begin(), wstring.end()};
}

//----------------------------------------------------------------------------
//! @brief stringをあるセパレート文字で分解します
//! @param string 分解したい文字列
//! @param separator 区切り文字
//! @return 分解した文字列のリスト
//----------------------------------------------------------------------------
std::vector<std::string> Split(std::string_view string, char separator)
{
    std::stringstream stream;
    stream << string;

    std::vector<std::string> result;

    std::string line;
    while(std::getline(stream, line, separator)) {
        result.emplace_back(std::move(line));
        line.clear();
    }

    return result;
}

//! @brief 文字列に「特定の文字列」が存在するかチェックする
//! @param str 文字列
//! @param v 含まれる可能性がある「特定の文字列」
//! @retval true : 含まれている
bool IsContains(std::string str, std::string_view v)
{
    if(str.find(v) != std::string::npos)
        return true;

    return false;
}

}   // namespace HelperLib::String

namespace HelperLib::OS
{
namespace
{
std::string GetRegString(HKEY hkey, const char* sub_key, const char* value_name)
{
    DWORD key_type  = 0;
    DWORD data_size = 0;
    if(ERROR_SUCCESS ==
       RegGetValue(hkey, (PCHAR)sub_key, (PCHAR)value_name, RRF_RT_REG_SZ, &key_type, nullptr, &data_size)) {
        std::string text;
        text.resize(data_size);

        if(ERROR_SUCCESS == RegGetValue(hkey,
                                        (PCHAR)sub_key,
                                        (PCHAR)value_name,
                                        RRF_RT_REG_SZ,
                                        &key_type,
                                        (PVOID)text.data(),
                                        &data_size)) {
            return text;
        }
    }
    return std::string();
}
}   // namespace

bool IsWindows11()
{
    auto reg = GetRegString(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", "CurrentBuild");

    int value = std::stoi(reg);
    if(value >= 22000)
        return true;

    return false;
}

bool IsWindows10OrGreater()
{
    std::string reg =
        GetRegString(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", "ProductName");

    if(String::IsContains(reg, "Windows 10"))
        return true;

    return false;
}
}   // namespace HelperLib::OS

namespace HelperLib::Math
{
bool NearlyEqual(float f1, float f2)
{
    if(fabs(f1 - f2) <= FLT_EPSILON)
        return true;

    return fabs(f1 - f2) <= FLT_EPSILON * std::max(fabs(f1), fabs(f2));
}

matrix CreateMatrixByFrontVector(float3 front, float3 up, bool upremake)
{
    float3 n_front = normalize(front);
    float3 n_up    = normalize(up);
    float3 n_right = cross(n_up, n_front);

    if(upremake) {
        n_up = normalize(cross(n_front, n_right));
    }

    matrix mat{
        float4{n_right, 0},
        float4{ n_up, 0 },
        float4{ n_front, 0      },
        float4{ 0, 0,                       0, 1},
    };
    return mat;
}

matrix LookAtMatrixForObject(float3 my_object_pos, float3 target_object_pos)
{
    auto mat      = CreateMatrixByFrontVector(target_object_pos - my_object_pos);
    mat._41_42_43 = my_object_pos;

    return mat;
}

}   // namespace HelperLib::Math
