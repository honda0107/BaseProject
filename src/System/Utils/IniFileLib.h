//---------------------------------------------------------------------------
//! @file   IniFileLib.h
//! @brief  Iniファイルライブラリ
//---------------------------------------------------------------------------
#pragma once

#include <System/Utils/HelperLib.h>
#include <string>

class IniFileLib
{
public:
    IniFileLib() {}
    virtual ~IniFileLib() {}

    //----------------------------------------------------------------------------
    //! @brief Iniファイル初期設定
    //! @param name ファイル名
    //----------------------------------------------------------------------------
    IniFileLib(const std::string& name) { File(name); }

    //! Iniファイルロード

    //----------------------------------------------------------------------------
    //! @brief Iniファイル名の設定
    //! @param name ファイル名
    //! @return ファイル名の存在
    //----------------------------------------------------------------------------
    bool File(const std::string& name)
    {
        file_ = "data/" + name;
        return HelperLib::File::CheckFileExistence(file_);
    }

    //! 整数値取得

    //----------------------------------------------------------------------------
    //! @brief 整数を取得します
    //! @param section セクション名
    //! @param key キー名
    //! @param def デフォルト数値(ないとき選ばれる)
    //! @return 数値
    //----------------------------------------------------------------------------
    int GetInt(const std::string& section, const std::string& key, int def = 0)
    {
        ReadBuffer(section.c_str(), key.c_str(), "None");
        const auto str = std::string(buffer_.data());
        if(str == "None")
            return def;

        return stoi(str);
    }

    //----------------------------------------------------------------------------
    //! @brief 浮動小数点数取得
    //! @param section セクション名
    //! @param key キー名
    //! @param def デフォルト数値(ないとき選ばれる)
    //! @return 数値
    //----------------------------------------------------------------------------
    float GetFloat(const std::string& section, const std::string& key, float def = 0.0f)
    {
        ReadBuffer(section.c_str(), key.c_str(), "None");
        const auto str = std::string(buffer_.data());
        if(str == "None")
            return def;

        return stof(str);
    }

    //----------------------------------------------------------------------------
    //! @brief 浮動小数点数取得(2要素)
    //! @param section セクション名
    //! @param key キー名
    //! @param def デフォルト数値(ないとき選ばれる)
    //! @return 数値
    //----------------------------------------------------------------------------
    float2 GetFloat2(const std::string& section, const std::string& key, float2 def = {0.0f, 0.0f})
    {
        ReadBuffer(section.c_str(), key.c_str(), "None");
        const auto str = std::string(buffer_.data());
        if(str == "None")
            return def;

        auto v2 = HelperLib::String::Split(str, ',');
        return float2(stof(v2[0]), stof(v2[1]));
    }

    //----------------------------------------------------------------------------
    //! @brief 浮動小数点数取得(3要素)
    //! @param section セクション名
    //! @param key キー名
    //! @param def デフォルト数値(ないとき選ばれる)
    //! @return 数値
    //----------------------------------------------------------------------------
    float3 GetFloat3(const std::string& section, const std::string& key, float3 def = {0, 0, 0})
    {
        ReadBuffer(section.c_str(), key.c_str(), "None");
        const auto str = std::string(buffer_.data());
        if(str == "None")
            return def;

        auto v2 = HelperLib::String::Split(str, ',');
        return float3(stof(v2[0]), stof(v2[1]), stof(v2[2]));
    }

    //----------------------------------------------------------------------------
    //! @brief 浮動小数点数取得(4要素)
    //! @param section セクション名
    //! @param key キー名
    //! @param def デフォルト数値(ないとき選ばれる)
    //! @return 数値
    //----------------------------------------------------------------------------
    float4 GetFloat4(const std::string& section, const std::string& key, float4 def = {0, 0, 0, 0})
    {
        ReadBuffer(section.c_str(), key.c_str(), "None");
        const auto str = std::string(buffer_.data());
        if(str == "None")
            return def;

        auto v2 = HelperLib::String::Split(str, ',');
        return float4(stof(v2[0]), stof(v2[1]), stof(v2[2]), stof(v2[3]));
    }

    //----------------------------------------------------------------------------
    //! @brief 文字列の取得
    //! @param section セクション名
    //! @param key キー名
    //! @param def デフォルト数値(ないとき選ばれる)
    //! @return 文字列
    //----------------------------------------------------------------------------
    std::string GetString(const std::string& section, const std::string& key, std::string def = "")
    {
        ReadBuffer(section.c_str(), key.c_str(), "None");
        const auto str = std::string(buffer_.data());
        if(str == "None")
            return def;

        return str;
    }

    //----------------------------------------------------------------------------
    //! @brief 文字列の取得
    //! @param section セクション名
    //! @param key キー名
    //! @param def デフォルト数値(ないとき選ばれる)
    //! @return 文字列
    //----------------------------------------------------------------------------
    std::vector<std::string> GetStrings(const std::string& section, const std::string& key)
    {
        ReadBuffer(section.c_str(), key.c_str(), "None");
        const auto str = std::string(buffer_.data());
        if(str == "None")
            return std::vector<std::string>();

        return HelperLib::String::Split(str, ',');
    }

private:
    //----------------------------------------------------------------------------
    //! @brief 文字列をbuffer_に読み込む
    //! @param section セッション
    //! @param key キー
    //! @param def デフォルト
    //----------------------------------------------------------------------------
    void ReadBuffer(LPCTSTR section, LPCTSTR key, LPCTSTR def)
    {
        GetPrivateProfileStringA(section, key, def, buffer_.data(), (DWORD)buffer_.size(), file_.c_str());
    }

    std::string           file_{};
    std::array<char, 256> buffer_{};
};
