//---------------------------------------------------------------------------
//! @file   Shader.cpp
//! @brief  シェーダー
//---------------------------------------------------------------------------
#include <mutex>

#include <d3d11.h>         // DirectX11
#include <d3dcompiler.h>   // シェーダーコンパイラ

#include "System/FileWatcher.h"
#include "Shader.h"

// DirectX関連のリンク
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

namespace
{

FileWatcher    file_watcher_;      //!< シェーダーホットリロード時のためのファイル監視
std::once_flag once_initialize_;   //!< 初回実行用フラグ

//! 現在アクティブなシェーダーリスト
//! @note 起動時の初期化順序の都合で固定長配列にしている
//! @note 個数が足りなくなった場合は適宜増やす
std::array<ShaderBase*, 1024> shaders_;
u32                           shader_count_;   // シェーダー個数
}   // namespace

//---------------------------------------------------------------------------
//! コンストラクタ
//---------------------------------------------------------------------------
ShaderBase::ShaderBase()
{
    assert(shader_count_ < shaders_.size());

    // シェーダーリストに登録
    shaders_[shader_count_] = this;
    shader_count_++;
}

//---------------------------------------------------------------------------
//! 作成
//---------------------------------------------------------------------------
ShaderBase::ShaderBase(std::string_view path, u32 type, u32 variant_count)
    : ShaderBase()
{
    assert(type <= DX_SHADERTYPE_HULL);

    //-----------------------------------------------------------------------
    // 初回実行時にファイル監視を開始する
    //-----------------------------------------------------------------------
    // ファイル変更時のコールバック関数
    auto file_modified_callback = [](const wchar_t* path) {
        // ファイルパスの要素を分解
        wchar_t drive[MAX_PATH];
        wchar_t dir[MAX_PATH];
        wchar_t name[MAX_PATH];
        wchar_t ext[MAX_PATH];

        _wsplitpath_s(path, drive, dir, name, ext);

        // シェーダーソースコードだった場合
        if(lstrcmpiW(ext, L".fx") == 0) {
            for(u32 i = 0; i < shader_count_; ++i) {
                auto& shader        = shaders_[i];
                auto  shader_path   = shader->path() + L".fx";
                auto  notified_path = std::wstring(path);

                // 小文字に統一することで大文字小文字の差異をなくす
                std::transform(shader_path.begin(),
                               shader_path.end(),
                               shader_path.begin(),
                               reinterpret_cast<wchar_t (*)(wchar_t)>(::tolower));
                std::transform(notified_path.begin(),
                               notified_path.end(),
                               notified_path.begin(),
                               reinterpret_cast<wchar_t (*)(wchar_t)>(::tolower));

                if(shader_path == notified_path) {
                    // 該当するシェーダーが見つかったら再コンパイル
                    shader->compile();
                }
            }
        }
    };

    // ファイル監視開始
    std::call_once(once_initialize_, [&]() { file_watcher_.initialize(L".", file_modified_callback); });

    //---------------------------------------------------------------------
    // 初期設定
    //---------------------------------------------------------------------
    path_ = convertTo(path);
    type_ = type;

    // 最低限1つのバリエーションを持つ
    if(variant_count == 0)
        variant_count = 1;

    handles_.resize(variant_count);

    for(auto& x : handles_) {
        x = -1;   // デフォルト値
    }

    compile();
}

//---------------------------------------------------------------------------
//! デストラクタ
//---------------------------------------------------------------------------
ShaderBase::~ShaderBase()
{
    // シェーダーリストから登録解除
    {
        auto it = std::find(std::begin(shaders_), std::begin(shaders_) + shader_count_, this);
        if(it != std::begin(shaders_) + shader_count_) {
            shader_count_--;
            *it = nullptr;
            std::swap(*it, shaders_[shader_count_]);
        }
    }

    // シェーダーを解放
    for(auto& x : handles_) {
        if(x != -1) {
            DeleteShader(x);
        }
    }
}

//---------------------------------------------------------------------------
//  ファイル監視を更新
//---------------------------------------------------------------------------
void ShaderBase::updateFileWatcher()
{
    file_watcher_.update();
}

//---------------------------------------------------------------------------
//! コンパイル実行
//---------------------------------------------------------------------------
bool ShaderBase::compile()
{
    // シェーダーソースの拡張子のパスを作成
    std::wstring source_path = path_ + L".fx";

    //---------------------------------------------------------------------
    // ファイルからソースファイルを読み込み
    //---------------------------------------------------------------------
    std::vector<std::byte> source;
    {
        // ファイルから読み込み
        std::ifstream file(source_path.c_str(),
                           std::ios::in | std::ios::binary |
                               std::ios::ate);   // ateを指定すると最初からファイルポインタが末尾に移動
        if(!file.is_open()) {
            return false;
        }
        auto size = file.tellg();
        source.resize(static_cast<size_t>(size));

        file.seekg(0, std::ios::beg);
        file.read(reinterpret_cast<char*>(source.data()), size);
        file.close();
    }

    //----------------------------------------------------------
    // シェーダーをコンパイル
    //----------------------------------------------------------
    u32 compile_flags = D3DCOMPILE_OPTIMIZATION_LEVEL3;
    compile_flags |= D3DCOMPILE_PACK_MATRIX_ROW_MAJOR;

#if defined(_DEBUG)
    // GraphicDebuggingツール用にシェーダーデバッグ向け設定
    compile_flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    //! シェーダーモデル名
    static const char* target_names[]{
        "vs_5_0",   // DX_SHADERTYPE_VERTEX   // 頂点シェーダー
        "ps_5_0",   // DX_SHADERTYPE_PIXEL    // ピクセルシェーダー
        "gs_5_0",   // DX_SHADERTYPE_GEOMETRY // ジオメトリシェーダー
        "cs_5_0",   // DX_SHADERTYPE_COMPUTE  // コンピュートシェーダー
        "ds_5_0",   // DX_SHADERTYPE_DOMAIN   // ドメインシェーダー
        "hs_5_0",   // DX_SHADERTYPE_HULL     // ハルシェーダー
    };

    auto compile_shader_variant = [&](u32 index) -> int {
        // #define SHADER_VARIANT  index の形式でプリプロセッサ定義
        auto number_string = std::to_string(index);

        const D3D_SHADER_MACRO defines[]{
            {"SHADER_VARIANT", number_string.c_str()},
            {         nullptr,               nullptr},
        };

        Microsoft::WRL::ComPtr<ID3DBlob> byte_code = nullptr;
        Microsoft::WRL::ComPtr<ID3DBlob> errors;

        auto hr =
            D3DCompile(source.data(),   // [in]  ソースコードのメモリ上のアドレス
                       source.size(),   // [in]  ソースコードサイズ
                       convertTo(source_path).c_str(),   // [in]  ソースコードのファイルパス(使用しない場合はnullptr)
                       defines,                          // [in]  プリプロセッサマクロ定義
                       D3D_COMPILE_STANDARD_FILE_INCLUDE,   // [in]  カスタムインクルード処理
                       "main",                              // [in]  関数名
                       target_names[type_],                 // [in]  シェーダーモデル名
                       compile_flags,                       // [in]  コンパイラフラグ  (D3DCOMPILE_xxxx)
                       0,                                   // [in]  コンパイラフラグ2 (D3DCOMPILE_FLAGS2_xxxx)
                       &byte_code,                          // [out] コンパイルされたバイトコード
                       &errors);                            // [out] エラーメッセージ

        // エラー警告出力
        if(errors != nullptr) {
            // 「出力」ウィンドウに表示
            OutputDebugStringA("--------------------\n");
            OutputDebugStringA((char*)errors->GetBufferPointer());
            OutputDebugStringA("--------------------\n");

            // メッセージボックス
            auto file_name = convertTo(source_path);
            MessageBox(DxLib::GetMainWindowHandle(),
                       static_cast<char*>(errors->GetBufferPointer()),
                       file_name.c_str(),
                       MB_ICONWARNING | MB_OK);
        }

        if(FAILED(hr)) {
            return -1;
        }

        //------------------------------------------------------
        // [DxLib] シェーダーを作成
        //------------------------------------------------------
        const void* shader_ptr = byte_code->GetBufferPointer();   // シェーダーバイナリの先頭アドレス
        auto shader_size = static_cast<int>(byte_code->GetBufferSize());   // シェーダーバイナリのサイズ
        int  handle      = -1;

        switch(type_) {
        case DX_SHADERTYPE_VERTEX:   // 頂点シェーダー
            handle = LoadVertexShaderFromMem(shader_ptr, shader_size);
            break;
        case DX_SHADERTYPE_PIXEL:   // ピクセルシェーダー
            handle = LoadPixelShaderFromMem(shader_ptr, shader_size);
            break;
        case DX_SHADERTYPE_GEOMETRY:   // ジオメトリシェーダー
            handle = LoadGeometryShaderFromMem(shader_ptr, shader_size);
            break;
        default:
            break;
        }

        return handle;
    };

    //----------------------------------------------------------
    // シェーダーバリエーションコンパイル
    //----------------------------------------------------------
    for(u32 i = 0; i < handles_.size(); i++) {
        // コンパイル
        auto handle = compile_shader_variant(i);

        // シェーダー作成が成功したら旧シェーダーを解放して置換
        if(handle == -1) {
            return false;
        }

        if(handles_[i] != -1) {
            DeleteShader(handles_[i]);
        }
        handles_[i] = handle;
    }

    return true;
}
