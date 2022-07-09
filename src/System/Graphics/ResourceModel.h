//---------------------------------------------------------------------------
//! @file   ResourceModel.h
//! @brief  3Dモデルリソース
//---------------------------------------------------------------------------
#pragma once

class ModelCache;   // 3Dモデルキャッシュ

//===========================================================================
//! 3Dモデルリソース
//===========================================================================
class ResourceModel final
{
public:
    // コンストラクタ
    ResourceModel(std::string_view path);

    // デストラクタ
    ~ResourceModel();

    // 読み込み完了まで待つ
    void waitForReadFinish();

    // [DxLib] MV1ハンドルを取得
    operator int() const;

    //! モデルキャッシュを取得
    ModelCache* modelCache() const;

    // ファイルパスを取得
    const std::wstring& path() const;

    // 初期化が正しく成功しているかどうか
    bool isValid() const;

    // 描画可能な状態かどうか取得
    //! @note   描画可能になっていない状態でMV1関数を呼ぶとブロッキングされます
    bool isActive() const;

private:
    //----------------------------------------------------------
    //! @name   copy/move禁止
    //----------------------------------------------------------
    //@{

    ResourceModel(const ResourceModel&)  = delete;
    ResourceModel(ResourceModel&&)       = delete;
    void operator=(const ResourceModel&) = delete;
    void operator=(ResourceModel&&)      = delete;

    //@}

private:
    int                         mv1_handle_ = -1;   //!< [DxLib] MV1モデルハンドル
    std::wstring                path_;              //!< モデルファイルへのパス
    std::atomic<bool>           active_ = false;   //!< アクティブ状態 true:利用可能 false:ロード未完了
    std::unique_ptr<ModelCache> model_cache_;      //!< 3Dモデルキャッシュ
};
