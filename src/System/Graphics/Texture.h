//---------------------------------------------------------------------------
//! @file   Texture.h
//! @brief  テクスチャ
//---------------------------------------------------------------------------
#pragma once

//===========================================================================
//! テクスチャ
//===========================================================================
class Texture
{
public:
    // コンストラクタ
    Texture() = default;

    // D3D11Resource指定で初期化
    //! @param  [in]    d3d_resource    D3Dリソースオブジェクト
    Texture(ID3D11Resource* d3d_resource);

    // [DxLib] Graphicハンドル指定で初期化
    //! @param  [in]    graphic_handle  DXライブラリのグラフィックハンドル
    Texture(int graphic_handle);

    // ファイルから作成
    //! @param [in] path    ファイルパス
    Texture(std::string_view path);

    // 描画対象のテクスチャ(RenderTarget/DepthStencil)を作成
    //! @param [in] width       幅
    //! @param [in] height      高さ
    //! @param [in] dxgi_format ピクセルフォーマット
    Texture(u32 width, u32 height, DXGI_FORMAT dxgi_format);

    // デストラクタ
    virtual ~Texture();

    // 解放
    void clear();

    //----------------------------------------------------------
    //! @note   取得/参照
    //----------------------------------------------------------
    //@{

    // 幅を取得
    u32 width() const;

    // 高さを取得
    u32 height() const;

    // D3Dリソースを取得
    ID3D11Resource* d3dResource() const;

    // ShaderResourceViewを取得
    ID3D11ShaderResourceView* srv() const;

    // RenderTargetViewを取得
    ID3D11RenderTargetView* rtv() const;

    // DepthStencilViewを取得
    ID3D11DepthStencilView* dsv() const;

    // [DxLib] Graphハンドルを取得
    operator int();

    // ファイルパスを取得
    const std::wstring& path() const;

    // 初期化が正しく成功しているかどうか
    bool is_valid() const;

    // 描画可能な状態かどうか取得
    //! @note   描画可能になっていない状態でMV1関数を呼ぶとブロッキングされます
    bool is_active() const;

    //@}

protected:
    // D3D11Resource指定で初期化
    bool initialize(ID3D11Resource* d3d_resource);

    // 遅延初期化
    void on_initialize();

protected:
    u32               width_  = 0;       //!< 幅
    u32               height_ = 0;       //!< 高さ
    int               handle_ = -1;      //!< [DxLib] Graphicハンドル
    std::wstring      path_;             //!< ファイルパス
    std::atomic<bool> active_ = false;   //!< アクティブ状態 true:利用可能 false:ロード未完了
    std::atomic<bool> need_initialize_ = false;   //!< 初期化要求フラグ true:初期化が必要 false:初期化済または完了で不要

    Microsoft::WRL::ComPtr<ID3D11Resource>           d3d_resource_;   //!< D3Dリソース(Texture2D/3D)
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> d3d_srv_;        //!< D3D ShaderResource View
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView>   d3d_rtv_;        //!< D3D RenderTarget   View
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView>   d3d_dsv_;        //!< D3D DepthStencil   View
};
