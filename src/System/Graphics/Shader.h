//---------------------------------------------------------------------------
//! @file   Shader.h
//! @brief  シェーダー
//---------------------------------------------------------------------------
#pragma once

//===========================================================================
//! シェーダー基底
//===========================================================================
class ShaderBase
{
public:
    // コンストラクタ
    ShaderBase();

    // デストラクタ
    virtual ~ShaderBase();

    // 作成
    //! @param [in] path            ファイルパス
    //! @param [in] type            シェーダーの種類(DX_SHADERTYPE_VERTEXなど)
    //! @param [in] variant_count    シェーダーバリエーション数(default:0)
    ShaderBase(std::string_view path, u32 type, u32 variant_count = 0);

    // コンパイル実行
    bool compile();

    //! [DxLib] シェーダーハンドルを取得
    operator int() const { return 0 < handles_.size() ? handles_[0] : -1; }

    //! [DxLib] シェーダーハンドルを取得(バリエーション指定)
    int variant(u32 index) const { return index < handles_.size() ? handles_[index] : -1; }

    //! ファイルパスを取得
    const std::wstring& path() const { return path_; }

    // ファイル監視を更新
    static void updateFileWatcher();

private:
    std::wstring     path_;        //!< ファイルパス
    std::vector<int> handles_;     //!< [DxLib] シェーダーハンドル
    int              type_ = -1;   //!< [DxLib] シェーダーの種類(DX_SHADERTYPE_VERTEXなど)
};

//===========================================================================
//! 頂点シェーダー
//===========================================================================
class ShaderVs final : public ShaderBase
{
public:
    ShaderVs(std::string_view path, u32 variant_count = 0)
        : ShaderBase(path, DX_SHADERTYPE_VERTEX, variant_count)
    {
    }

private:
};

//===========================================================================
//! ピクセルシェーダー
//===========================================================================
class ShaderPs final : public ShaderBase
{
public:
    ShaderPs(std::string_view path, u32 variant_count = 0)
        : ShaderBase(path, DX_SHADERTYPE_PIXEL, variant_count)
    {
    }
};

//===========================================================================
//! ジオメトリシェーダー
//===========================================================================
class ShaderGs final : public ShaderBase
{
public:
    ShaderGs(std::string_view path, u32 variant_count = 0)
        : ShaderBase(path, DX_SHADERTYPE_GEOMETRY, variant_count)
    {
    }
};
