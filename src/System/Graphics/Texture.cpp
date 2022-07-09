//---------------------------------------------------------------------------
//! @file   Texture.cpp
//! @brief  テクスチャ
//---------------------------------------------------------------------------
#include "Texture.h"

//---------------------------------------------------------------------------
//! デストラクタ
//---------------------------------------------------------------------------
Texture::~Texture()
{
    clear();
}

//---------------------------------------------------------------------------
//! 解放
//---------------------------------------------------------------------------
void Texture::clear()
{
    if(handle_ != -1) {
        SetASyncLoadFinishDeleteFlag(handle_);
        handle_ = -1;
    }

    d3d_resource_.Reset();
    d3d_srv_.Reset();
    d3d_rtv_.Reset();
    d3d_dsv_.Reset();
}

//---------------------------------------------------------------------------
//!  D3D11Resource指定で初期化
//---------------------------------------------------------------------------
Texture::Texture(ID3D11Resource* d3d_resource)
{
    active_ = true;
    initialize(d3d_resource);
}

//---------------------------------------------------------------------------
//!  [DxLib] ハンドル指定で初期化
//---------------------------------------------------------------------------
Texture::Texture(int grHandle)
{
    active_ = true;
    handle_ = grHandle;

    // 画像から幅と高さを取得
    int w, h;
    GetGraphSize(handle_, &w, &h);
    width_  = static_cast<u32>(w);
    height_ = static_cast<u32>(h);

    //----------------------------------------------------------
    // D3Dリソース初期化
    //----------------------------------------------------------
    d3d_resource_ = reinterpret_cast<ID3D11Resource*>(const_cast<void*>(GetGraphID3D11Texture2D(handle_)));
    d3d_rtv_ = reinterpret_cast<ID3D11RenderTargetView*>(const_cast<void*>(GetGraphID3D11RenderTargetView(handle_)));
    d3d_dsv_ = reinterpret_cast<ID3D11DepthStencilView*>(const_cast<void*>(GetGraphID3D11DepthStencilView(handle_)));
}

//---------------------------------------------------------------------------
//!  ファイルから作成
//---------------------------------------------------------------------------
Texture::Texture(std::string_view path)
{
    //-----------------------------------------------------------------------
    // テクスチャの読み込み
    //-----------------------------------------------------------------------
    auto texture_path = std::string(path);

    // パスの保存
    path_ = convertTo(texture_path);

    // ハンドルの非同期読み込み処理が完了したら呼ばれる関数
    auto finish_callback = []([[maybe_unused]] int handle, void* data) {
        Texture* texture = reinterpret_cast<Texture*>(data);

        // アクティブフラグを設定
        texture->need_initialize_ = true;
        texture->active_          = true;
    };

    // 非同期ロード
    SetUseASyncLoadFlag(true);
    handle_ = LoadGraph(texture_path.c_str());
    SetASyncLoadFinishCallback(handle_, (void (*)(int, void*))finish_callback, this);
    SetUseASyncLoadFlag(false);
}

//---------------------------------------------------------------------------
//!  描画対象のテクスチャ(RenderTarget/DepthStencil)を作成
//---------------------------------------------------------------------------
Texture::Texture(u32 width, u32 height, DXGI_FORMAT dxgi_format)
{
    D3D11_TEXTURE2D_DESC desc{};
    desc.Width          = width;                        // 幅
    desc.Height         = height;                       // 高さ
    desc.MipLevels      = 1;                            // ミップレベル段数
    desc.ArraySize      = 1;                            // 配列数
    desc.Format         = dxgi_format;                  // ピクセルフォーマット
    desc.SampleDesc     = {1, 0};                       // マルチサンプルOFF
    desc.Usage          = D3D11_USAGE_DEFAULT;          // GPUメモリに配置
    desc.BindFlags      = D3D11_BIND_SHADER_RESOURCE;   // バインド用途(テクスチャ)
    desc.CPUAccessFlags = 0;                            // CPUアクセス禁止
    desc.MiscFlags      = 0;                            // フラグなし

    // デプスバッファの場合はR32としてメモリ初期化
    if(dxgi_format == DXGI_FORMAT_D32_FLOAT) {
        desc.Format = DXGI_FORMAT_R32_TYPELESS;
        desc.BindFlags |= D3D11_BIND_DEPTH_STENCIL;
    }
    else {
        desc.BindFlags |= D3D11_BIND_RENDER_TARGET;
    }

    //-----------------------------------------------------
    // D3Dテクスチャを作成
    //-----------------------------------------------------
    auto* d3d_device = reinterpret_cast<ID3D11Device*>(const_cast<void*>(GetUseDirect3D11Device()));

    Microsoft::WRL::ComPtr<ID3D11Texture2D> d3d_texture_2d;
    d3d_device->CreateTexture2D(&desc, nullptr, &d3d_texture_2d);

    // 初期化
    initialize(d3d_texture_2d.Get());

    active_ = true;
}

//---------------------------------------------------------------------------
//!  D3D11Resource指定で初期化
//---------------------------------------------------------------------------
bool Texture::initialize(ID3D11Resource* d3d_resource)
{
    if(d3d_resource == nullptr)
        return false;

    d3d_resource_ = d3d_resource;

    // GPUリソースの種類によって処理
    // 現時点ではTexture2Dのみ
    D3D11_RESOURCE_DIMENSION dimension;
    d3d_resource->GetType(&dimension);

    if(dimension != D3D11_RESOURCE_DIMENSION_TEXTURE2D) {
        assert(0);   // Texture2D以外は未対応
        return false;
    }

    //----------------------------------------------------------
    // ビューを初期化
    //----------------------------------------------------------
    {
        auto* d3d_device     = reinterpret_cast<ID3D11Device*>(const_cast<void*>(GetUseDirect3D11Device()));
        auto* d3d_texture_2d = reinterpret_cast<ID3D11Texture2D*>(d3d_resource);

        D3D11_TEXTURE2D_DESC desc;
        d3d_texture_2d->GetDesc(&desc);

        width_  = desc.Width;    // 幅
        height_ = desc.Height;   // 高さ

        if(desc.BindFlags & D3D11_BIND_SHADER_RESOURCE) {
            if(desc.Format == DXGI_FORMAT_R32_TYPELESS) {   // デプスバッファ用の場合はR32_FLOATとして利用
                D3D11_SHADER_RESOURCE_VIEW_DESC view_desc{};
                view_desc.ViewDimension       = D3D11_SRV_DIMENSION_TEXTURE2D;
                view_desc.Format              = DXGI_FORMAT_R32_FLOAT;
                view_desc.Texture2D.MipLevels = 1;
                d3d_device->CreateShaderResourceView(d3d_resource, &view_desc, &d3d_srv_);
            }
            else {
                d3d_device->CreateShaderResourceView(d3d_resource, nullptr, &d3d_srv_);

                // ID3D11Texture2DからDxLibグラフィックハンドルを作成
                if(handle_ == -1) {
                    handle_ = CreateGraphFromID3D11Texture2D(d3d_resource);
                }
            }
        }

        if(desc.BindFlags & D3D11_BIND_RENDER_TARGET) {
            d3d_device->CreateRenderTargetView(d3d_resource, nullptr, &d3d_rtv_);
        }

        if(desc.BindFlags & D3D11_BIND_DEPTH_STENCIL) {
            D3D11_DEPTH_STENCIL_VIEW_DESC view_desc{};
            view_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
            if(desc.Format == DXGI_FORMAT_R16_TYPELESS) {
                view_desc.Format = DXGI_FORMAT_D16_UNORM;
            }
            else if(desc.Format == DXGI_FORMAT_D32_FLOAT || desc.Format == DXGI_FORMAT_R32_TYPELESS) {
                view_desc.Format = DXGI_FORMAT_D32_FLOAT;
            }
            else {
                view_desc.Format = DXGI_FORMAT_UNKNOWN;
            }
            d3d_device->CreateDepthStencilView(d3d_resource, &view_desc, &d3d_dsv_);
        }
    }
    return true;
}

//---------------------------------------------------------------------------
//! 遅延初期化
//---------------------------------------------------------------------------
void Texture::on_initialize()
{
    if(need_initialize_ == false)
        return;

    //----------------------------------------------------------
    // D3Dリソース初期化
    //----------------------------------------------------------
    auto* d3d_resource = reinterpret_cast<ID3D11Resource*>(const_cast<void*>(GetGraphID3D11Texture2D(handle_)));

    initialize(d3d_resource);
    need_initialize_ = false;
}

//---------------------------------------------------------------------------
//!  幅を取得
//---------------------------------------------------------------------------
u32 Texture::width() const
{
    return width_;
}

//---------------------------------------------------------------------------
//!  高さを取得
//---------------------------------------------------------------------------
u32 Texture::height() const
{
    return height_;
}

//---------------------------------------------------------------------------
//! D3Dリソースを取得
//---------------------------------------------------------------------------
ID3D11Resource* Texture::d3dResource() const
{
    return d3d_resource_.Get();
}

//---------------------------------------------------------------------------
//! ShaderResourceViewを取得
//---------------------------------------------------------------------------
ID3D11ShaderResourceView* Texture::srv() const
{
    return d3d_srv_.Get();
}

//---------------------------------------------------------------------------
//! RenderTargetViewを取得
//---------------------------------------------------------------------------
ID3D11RenderTargetView* Texture::rtv() const
{
    return d3d_rtv_.Get();
}

//---------------------------------------------------------------------------
//! DepthStencilViewを取得
//---------------------------------------------------------------------------
ID3D11DepthStencilView* Texture::dsv() const
{
    return d3d_dsv_.Get();
}

//---------------------------------------------------------------------------
//! [DxLib] Graphハンドルを取得
//---------------------------------------------------------------------------
Texture::operator int()
{
    // 非同期ロード完了チェック
    if(!active_) {
        return DX_NONE_GRAPH;
    }

    // 初期化チェック
    on_initialize();

    return handle_ <= 0 ? DX_NONE_GRAPH : handle_;
}

//---------------------------------------------------------------------------
//! ファイルパスを取得
//---------------------------------------------------------------------------
const std::wstring& Texture::path() const
{
    return path_;
}

//---------------------------------------------------------------------------
//! 初期化が正しく成功しているかどうか
//---------------------------------------------------------------------------
bool Texture::is_valid() const
{
    return d3d_resource_ || handle_ != -1;
}

//---------------------------------------------------------------------------
//! 描画可能な状態かどうか取得
//---------------------------------------------------------------------------
bool Texture::is_active() const
{
    return active_;
}
