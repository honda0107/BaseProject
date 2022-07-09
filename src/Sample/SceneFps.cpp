//---------------------------------------------------------------------------
//! @file   SceneFps.cpp
//! @brief  First Person Shooting サンプルシーン
//---------------------------------------------------------------------------
#include "SceneFps.h"
#include <System/Component/ComponentModel.h>
#include <System/Component/ComponentCamera.h>
#include <System/Graphics/Animation.h>

#include <System/SystemMain.h>   // ShowGrid

BP_CLASS_IMPL(SceneFps, u8"FPSサンプル")

namespace
{

//-------------------------------------------------------
//! アニメーション定義
//-------------------------------------------------------
// {アニメーション名, ファイル名, ファイル内のアニメーション番号, 再生速度}
const Animation::Desc desc_[] = {
    {         "clap", "data/Sample/FPS_Hands/FPS_Hands_v2.9x_animation.mv1",  1, 1.0f},
    {    "from_clap", "data/Sample/FPS_Hands/FPS_Hands_v2.9x_animation.mv1",  1, 1.0f},
    {    "from_shot", "data/Sample/FPS_Hands/FPS_Hands_v2.9x_animation.mv1",  2, 1.0f},
    { "from_warm-up", "data/Sample/FPS_Hands/FPS_Hands_v2.9x_animation.mv1",  3, 1.0f},
    {   "hide_fists", "data/Sample/FPS_Hands/FPS_Hands_v2.9x_animation.mv1",  4, 1.0f},
    {     "hide_gun", "data/Sample/FPS_Hands/FPS_Hands_v2.9x_animation.mv1",  5, 1.0f},
    {   "hide_knife", "data/Sample/FPS_Hands/FPS_Hands_v2.9x_animation.mv1",  6, 1.0f},
    {         "like", "data/Sample/FPS_Hands/FPS_Hands_v2.9x_animation.mv1",  7, 1.0f},
    {      "PoseLib", "data/Sample/FPS_Hands/FPS_Hands_v2.9x_animation.mv1",  8, 1.0f},
    {        "punch", "data/Sample/FPS_Hands/FPS_Hands_v2.9x_animation.mv1",  9, 1.0f},
    {         "shot", "data/Sample/FPS_Hands/FPS_Hands_v2.9x_animation.mv1", 10, 1.0f},
    {    "show_fist", "data/Sample/FPS_Hands/FPS_Hands_v2.9x_animation.mv1", 11, 1.0f},
    {     "show_gun", "data/Sample/FPS_Hands/FPS_Hands_v2.9x_animation.mv1", 12, 1.0f},
    {   "show_knife", "data/Sample/FPS_Hands/FPS_Hands_v2.9x_animation.mv1", 13, 1.0f},
    {         "stab", "data/Sample/FPS_Hands/FPS_Hands_v2.9x_animation.mv1", 14, 1.0f},
    {  "stand_fists", "data/Sample/FPS_Hands/FPS_Hands_v2.9x_animation.mv1", 15, 1.0f},
    {    "stand_gun", "data/Sample/FPS_Hands/FPS_Hands_v2.9x_animation.mv1", 16, 1.0f},
    {  "stand_knife", "data/Sample/FPS_Hands/FPS_Hands_v2.9x_animation.mv1", 17, 1.0f},
    {      "to_clap", "data/Sample/FPS_Hands/FPS_Hands_v2.9x_animation.mv1", 18, 1.0f},
    {      "to_shot", "data/Sample/FPS_Hands/FPS_Hands_v2.9x_animation.mv1", 19, 1.0f},
    {   "to_warm-up", "data/Sample/FPS_Hands/FPS_Hands_v2.9x_animation.mv1", 20, 1.0f},
    {"walking_fists", "data/Sample/FPS_Hands/FPS_Hands_v2.9x_animation.mv1", 21, 1.0f},
    {  "walking_gun", "data/Sample/FPS_Hands/FPS_Hands_v2.9x_animation.mv1", 22, 1.0f},
    {"walking_knife", "data/Sample/FPS_Hands/FPS_Hands_v2.9x_animation.mv1", 23, 1.0f},
    {      "warm-up", "data/Sample/FPS_Hands/FPS_Hands_v2.9x_animation.mv1", 24, 1.0f},
};

}   // namespace

//---------------------------------------------------------------------------
//! 初期化
//---------------------------------------------------------------------------
bool SceneFps::Init()
{
    // グリッド表示をOFF
    ShowGrid(false);

    //-------------------------------------------------------
    // 照準
    //-------------------------------------------------------
    tex_crosschairs_ = std::make_shared<Texture>("data/Sample/FPS_Crosschairs/crosshairs64.png");

    //-------------------------------------------------------
    // モデル
    //-------------------------------------------------------
    // FPS腕モデル
    {
        model_ = std::make_shared<Model>("data/Sample/FPS_Hands/FPS_Hands_v2.9x.mv1");

        // テクスチャ
        tex_diffuse_  = std::make_shared<Texture>("data/Sample/FPS_Hands/Textures_V2/Diffuse_V2.dds");
        tex_specular_ = std::make_shared<Texture>("data/Sample/FPS_Hands/Textures_V2/Specular_V2.dds");
        tex_albedo_   = std::make_shared<Texture>("data/Sample/FPS_Hands/Textures_V2/Albedo_V2.dds");
        tex_normal_   = std::make_shared<Texture>("data/Sample/FPS_Hands/Textures_V2/Normal_map_V2.dds");
        tex_ao_       = std::make_shared<Texture>("data/Sample/FPS_Hands/Textures_V2/AO.dds");

        // モデルに設定されているテクスチャを上書き
        model_->overrideTexture(Model::TextureType::Diffuse, tex_diffuse_);
        model_->overrideTexture(Model::TextureType::Specular, tex_specular_);
        model_->overrideTexture(Model::TextureType::Albedo, tex_albedo_);
        model_->overrideTexture(Model::TextureType::Normal, tex_normal_);
        model_->overrideTexture(Model::TextureType::AO, tex_ao_);
    }
    // タクティカルナイフ
    {
        model_knife_ = std::make_shared<Model>("data/Sample/FPS_Knife/Knife_low.mv1");

        // テクスチャ
        tex_kn0_albedo_    = std::make_shared<Texture>("data/Sample/FPS_Knife/Knife_low_Iron.001_BaseColor.png");
        tex_kn0_normal_    = std::make_shared<Texture>("data/Sample/FPS_Knife/Knife_low_Iron.001_NormalOpenGLl.png");
        tex_kn0_roughness_ = std::make_shared<Texture>("data/Sample/FPS_Knife/Knife_low_Iron.001_Roughness.png");
        tex_kn0_metalness_ = std::make_shared<Texture>("data/Sample/FPS_Knife/Knife_low_Iron.001_Metallic.png");

        tex_kn1_albedo_    = std::make_shared<Texture>("data/Sample/FPS_Knife/Knife_low_Lever.001_BaseColor.png");
        tex_kn1_normal_    = std::make_shared<Texture>("data/Sample/FPS_Knife/Knife_low_Lever.001_NormalOpenGLl.png");
        tex_kn1_roughness_ = std::make_shared<Texture>("data/Sample/FPS_Knife/Knife_low_Lever.001_Roughness.png");
        tex_kn1_metalness_ = std::make_shared<Texture>("data/Sample/FPS_Knife/Knife_low_Lever.001_Metallic.png");
    }

    // 背景
    model_field_ = std::make_shared<Model>("data/Sample/FPS_Industrial/Map.mv1");

    //-------------------------------------------------------
    // アニメーション
    //-------------------------------------------------------

    // アニメーション初期化
    animation_ = std::make_shared<Animation>(desc_, std::size(desc_));

    //  モデルにアニメーションを設定
    model_->bindAnimation(animation_.get());

    //-------------------------------------------------------
    // アニメーションを再生
    //-------------------------------------------------------
    bool is_loop = true;
    animation_->play("stand_knife", is_loop);

    return true;
}

//---------------------------------------------------------------------------
//! 更新
//! @param  [in]    delta   経過時間
//---------------------------------------------------------------------------
void SceneFps::Update([[maybe_unused]] float delta)
{
    // アニメーション再生時間を進める
    animation_->update(delta);

    //----------------------------------------------------------
    // マウス視点操作
    //----------------------------------------------------------
    float3     eye_position{-15.0f, 1.5f, 0.0f};
    static f32 rotate_v = 0.0f;
    static f32 rotate_h = 0.0f;

    {
        // マウスカーソル座標(ウインドウのクライアント領域の座標)を取得
        s32 x, y;
        GetMousePoint(&x, &y);

        static s32 mouse_x = x;
        static s32 mouse_y = y;

        // 移動量を差分で計算
        s32 diff_x = x - mouse_x;
        s32 diff_y = y - mouse_y;

        if(IsMouseRepeat(MOUSE_INPUT_RIGHT)) {   // 右クリックしている間
            // 視点を回転させる
            rotate_h += static_cast<f32>(diff_x) * 0.005f;
            rotate_v += static_cast<f32>(diff_y) * 0.005f;

            // 上下可動範囲制限
            rotate_v = std::clamp(rotate_v, -60.0f * DegToRad, +89.0f * DegToRad);

            // マウスカーソル位置を画面中央に戻す
            SetMousePoint(WINDOW_W / 2, WINDOW_H / 2);
        }

        // 値を次のフレームのために保存
        GetMousePoint(&mouse_x, &mouse_y);
    }

    //----------------------------------------------------------
    // FPS用のカメラを設定
    // 左右振り向き角度と上下角度で指定
    //----------------------------------------------------------

    // カメラのワールド行列
    matrix mat_camera = matrix::identity();
    {
        mat_camera = mul(mat_camera, matrix::rotateX(rotate_v));
        mat_camera = mul(mat_camera, matrix::rotateY(rotate_h));
        mat_camera = mul(mat_camera, matrix::translate(eye_position));
    }

    // ビュー行列 = カメラのワールド行列の逆行列
    matrix mat_view = inverse(mat_camera);

    // ビュー行列を設定
    SetCameraViewMatrix(cast(mat_view));

    // 投影行列をパラメーターで設定
    SetupCamera_Perspective(60.0f * DegToRad);   // 画角=60°
    SetCameraNearFar(0.01f, 1000.0f);

    //----------------------------------------------------------
    // 腕モデルの表示位置
    //----------------------------------------------------------
    {
        // 相対位置
        matrix mat_world = matrix::translate(float3(0.0f, -0.3f, +0.25f));

        // 腕モデルをカメラに装備させる (装備させたいワールド行列を後ろから掛ける)
        mat_world = mul(mat_world, mat_camera);

        model_->setWorldMatrix(mat_world);

        MV1SetMatrix(*model_, mat_world);
    }
    //----------------------------------------------------------
    // タクティカルナイフの表示位置
    //----------------------------------------------------------
    {
        // 武器をアタッチするフレームの番号を検索
        auto attach_frame = MV1SearchFrame(*model_, "hand.R");
        // 武器をアタッチするフレームのワールド行列を取得する
        auto mat_weapon_frame = MV1GetFrameLocalWorldMatrix(*model_, attach_frame);

        // 手にアタッチするだけでは位置が合わないため位置を調整
        matrix mat_world = matrix::scale(0.15f);

        mat_world = mul(mat_world, matrix::rotateY(-90.0f * DegToRad));                  // 方向調整
        mat_world = mul(mat_world, matrix::rotateX(180.0f * DegToRad));                  // 方向調整
        mat_world = mul(mat_world, matrix::translate(float3(-0.225f, 0.25f, 0.075f)));   // 位置調整

        // 手に装備 (装備したい関節フレームの行列を後ろから掛ける)
        mat_world = mul(mat_world, cast(mat_weapon_frame));

        model_knife_->setWorldMatrix(mat_world);
    }

    //---------------------------------------------------------
    // 攻撃
    //---------------------------------------------------------
    if(is_attacked_) {
        // 攻撃アニメーションが終わったら元に戻す
        if(animation_->isPlaying() == false) {
            is_attacked_ = false;                    // 攻撃終了
            animation_->play("stand_knife", true);   // 元に戻す
        }
    }
    else {
        // 左クリックしたら攻撃
        if(IsMouseRepeat(1)) {
            animation_->play("stab", false, 0.125f);   // 攻撃アニメーション(動作が短いため高速に補間する)
            is_attacked_ = true;                       // 攻撃中
        }
    }
}

//---------------------------------------------------------------------------
//! 描画
//---------------------------------------------------------------------------
void SceneFps::Draw()
{
    //----------------------------------------------------------
    // 背景を描画
    //----------------------------------------------------------
    model_field_->render();

    //----------------------------------------------------------
    // タクティカルナイフ描画
    //----------------------------------------------------------
    {
        // モデルに設定されているテクスチャを上書き
        model_knife_->overrideTexture(Model::TextureType::Diffuse, tex_kn0_albedo_);
        model_knife_->overrideTexture(Model::TextureType::Albedo, tex_kn0_albedo_);
        model_knife_->overrideTexture(Model::TextureType::Normal, tex_kn0_normal_);
        model_knife_->overrideTexture(Model::TextureType::Roughness, tex_kn0_roughness_);
        model_knife_->overrideTexture(Model::TextureType::Metalness, tex_kn0_metalness_);

        model_knife_->renderByMesh(0);   // 金属刃物部分

        model_knife_->overrideTexture(Model::TextureType::Diffuse, tex_kn1_albedo_);
        model_knife_->overrideTexture(Model::TextureType::Albedo, tex_kn1_albedo_);
        model_knife_->overrideTexture(Model::TextureType::Normal, tex_kn1_normal_);
        model_knife_->overrideTexture(Model::TextureType::Roughness, tex_kn1_roughness_);
        model_knife_->overrideTexture(Model::TextureType::Metalness, tex_kn1_metalness_);

        model_knife_->renderByMesh(1);   // 柄部分
    }
    //----------------------------------------------------------
    // FPS腕モデル描画
    //----------------------------------------------------------
    {
        model_->renderByMesh(0);   // 腕
                                   //model_->renderByMesh(1);   // ハンドガン(ダミー)
                                   //model_->renderByMesh(2);   // ナイフ(ダミー)
    }

    //----------------------------------------------------------
    // 照準を描画
    //----------------------------------------------------------
    {
        u32 type = 18;   // 種類(0-63)
        u32 size = 64;   // 画像イメージサイズ

        u32 source_x = size * (type & 7);
        u32 source_y = size * (type >> 3);

        // 加算半透明
        SetDrawBlendMode(DX_BLENDMODE_ADD, 128);

        DrawRectGraph((WINDOW_W - size) / 2,   // X座標
                      (WINDOW_H - size) / 2,   // Y座標
                      source_x,                // 元画像のX座標
                      source_y,                // 元画像のY座標
                      size,                    // 幅
                      size,                    // 高さ
                      *tex_crosschairs_,       // [DxLib] Graphハンドル
                      true);                   // true:半透明 false:不透明

        // 半透明OFF
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
    }
}

//---------------------------------------------------------------------------
//! 終了
//---------------------------------------------------------------------------
void SceneFps::Exit()
{
    // グリッド表示をON
    ShowGrid(true);
}

//---------------------------------------------------------------------------
//! GUI表示
//---------------------------------------------------------------------------
void SceneFps::GUI()
{
}
