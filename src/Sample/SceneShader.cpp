//---------------------------------------------------------------------------
//! @file   SceneShader.cpp
//! @brief  シェーダーサンプルシーン
//---------------------------------------------------------------------------
#include "SceneShader.h"
#include <System/Component/ComponentModel.h>
#include <System/Component/ComponentCamera.h>
#include <System/Component/ComponentFilterFade.h>

BP_CLASS_IMPL(SceneShader, u8"シェーダーサンプル");

//---------------------------------------------------------------------------
//! 初期化
//---------------------------------------------------------------------------
bool SceneShader::Init()
{
    auto obj = Scene::CreateObject<Object>();

    // モデルコンポーネント
    model_ = obj->AddComponent<ComponentModel>("data/Sample/Player/model.mv1");

    // カメラコンポーネント
    auto camera = obj->AddComponent<ComponentCamera>();
    camera->SetPerspective(60.0f);
    camera->SetPositionAndTarget({0.f, 6.f, -15.f}, {0.f, 5.f, 0.f});
    // camera->SetCurrentCamera(); //< こちらは1つめのカメラの場合は必要ありません

    // フェードコンポーネント
    filter_fade_ = obj->AddComponent<ComponentFilterFade>();

    // 画像の読み込み
    texture_ = std::make_shared<Texture>("data/Shader/seafloor.dds");

    // 頂点シェーダー
    shader_vs_ = std::make_shared<ShaderVs>("data/Shader/vs_3d");

    // ピクセルシェーダー
    shader_ps_    = std::make_shared<ShaderPs>("data/Shader/ps_texture");
    shader_ps_3d_ = std::make_shared<ShaderPs>("data/Shader/ps_3d_texture");

    return true;
}

//---------------------------------------------------------------------------
//! 更新
//! @param  [in]    delta   経過時間
//---------------------------------------------------------------------------
void SceneShader::Update([[maybe_unused]] float delta)
{
    // Y軸中心の回転
    static f32 ry = 0.0f;
    ry += 0.5f * delta;

    model_->Matrix() = mul(matrix::scale(0.1f), matrix::rotateY(ry));
}

//---------------------------------------------------------------------------
//! 描画
//---------------------------------------------------------------------------
void SceneShader::Draw()
{
    DrawFormatString(100, 50, GetColor(255, 255, 255), "ShaderDemo");

    //==========================================================
    // プリミティブの描画
    //==========================================================

    SetDrawMode(DX_DRAWMODE_BILINEAR);           // テクスチャをバイリニア補間
    SetTextureAddressMode(DX_TEXADDRESS_WRAP);   // テクスチャを繰り返し

    //----------------------------------------------------------
    // 床の描画
    //----------------------------------------------------------
    {
        VERTEX3DSHADER v[4]{};

        constexpr f32 SIZE = 5.0f;

        // 頂点データの準備
        v[0].pos = {-SIZE, 0.0f, +SIZE};
        v[1].pos = {+SIZE, 0.0f, +SIZE};
        v[2].pos = {-SIZE, 0.0f, -SIZE};
        v[3].pos = {+SIZE, 0.0f, -SIZE};
        v[0].dif = GetColorU8(255, 255, 255, 255);
        v[1].dif = GetColorU8(255, 255, 255, 255);
        v[2].dif = GetColorU8(255, 255, 255, 255);
        v[3].dif = GetColorU8(255, 255, 255, 255);
        v[0].u   = 0.0f;
        v[0].v   = 0.0f;
        v[1].u   = 2.0f;
        v[1].v   = 0.0f;
        v[2].u   = 0.0f;
        v[2].v   = 2.0f;
        v[3].u   = 2.0f;
        v[3].v   = 2.0f;

        // 使用するテクスチャを設定 (slot=0)
        SetUseTextureToShader(0, *texture_);

        // 使用する頂点シェーダーを設定
        SetUseVertexShader(*shader_vs_);

        // 使用するピクセルシェーダーを設定
        SetUsePixelShader(*shader_ps_3d_);

        // 描画
        DrawPrimitive3DToShader(v, 4, DX_PRIMTYPE_TRIANGLESTRIP);
    }
    // 使い終わったらテクスチャ設定を解除
    // 解除しておかないとモデル描画に影響あり。
    SetUseTextureToShader(0, -1);

    //----------------------------------------------------------
    // 2Dの描画
    //----------------------------------------------------------
    {
        VERTEX2DSHADER v[4]{};

        constexpr u32 SIZE = 256;

        // 頂点データの準備
        v[0].pos = {0.0f, 0.0f, 0.0f};
        v[1].pos = {SIZE, 0.0f, 0.0f};
        v[2].pos = {0.0f, SIZE, 0.0f};
        v[3].pos = {SIZE, SIZE, 0.0f};
        v[0].rhw = 1.0f;   // rhw = 1.0f 初期化は2D描画に必須
        v[1].rhw = 1.0f;
        v[2].rhw = 1.0f;
        v[3].rhw = 1.0f;
        v[0].dif = GetColorU8(255, 255, 255, 255);
        v[1].dif = GetColorU8(255, 255, 255, 255);
        v[2].dif = GetColorU8(255, 255, 255, 255);
        v[3].dif = GetColorU8(255, 255, 255, 255);
        v[0].u   = 0.0f;
        v[0].v   = 0.0f;
        v[1].u   = 1.0f;
        v[1].v   = 0.0f;
        v[2].u   = 0.0f;
        v[2].v   = 1.0f;
        v[3].u   = 1.0f;
        v[3].v   = 1.0f;

        // 使用するテクスチャを設定 (slot=0)
        SetUseTextureToShader(0, *texture_);

        // 使用するピクセルシェーダーを設定 (2Dの場合は頂点シェーダー不要)
        SetUsePixelShader(*shader_ps_);

        // 描画
        DrawPrimitive2DToShader(v, 4, DX_PRIMTYPE_TRIANGLESTRIP);
    }
    // 使い終わったらテクスチャ設定を解除
    // 解除しておかないとモデル描画に影響あり。
    SetUseTextureToShader(0, -1);
}

//---------------------------------------------------------------------------
//! 終了
//---------------------------------------------------------------------------
void SceneShader::Exit()
{
}

//---------------------------------------------------------------------------
//! GUI表示
//---------------------------------------------------------------------------
void SceneShader::GUI()
{
}
