// プリコンパイルヘッダービルド用cpp
// ここには実装を追加しない
#include "Precompile.h"

//---------------------------------------------------------------------------
// std::stringをstd::wstringへ変換する
//---------------------------------------------------------------------------
std::wstring convertTo(std::string_view string)
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

//---------------------------------------------------------------------------
// std::wstringをstd::stringへ変換する
//---------------------------------------------------------------------------
std::string convertTo(std::wstring_view wstring)
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
