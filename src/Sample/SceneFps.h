//---------------------------------------------------------------------------
//! @file   SceneFps.h
//! @brief  First Person Shooting サンプルシーン
//---------------------------------------------------------------------------
#pragma once

#include <System/Scene.h>

class Model;
class Animation;
class Texture;

//===========================================================================
//! FPSサンプルシーン
//===========================================================================
class SceneFps final : public Scene::Base
{
public:
    BP_CLASS_TYPE(SceneFps, Scene::Base)

    //! シーン名称
    std::string Name() override { return u8"FPSサンプル"; }

    bool Init() override;                //!< 初期化
    void Update(float delta) override;   //!< 更新
    void Draw() override;                //!< 描画
    void Exit() override;                //!< 終了
    void GUI() override;                 //!< GUI表示

private:
    bool is_attacked_ = false;   //!< 攻撃中かどうかのフラグ

    std::shared_ptr<Model> model_field_;   //!< 背景3Dモデル

    std::shared_ptr<Model>     model_;          //!< 3Dモデル
    std::shared_ptr<Animation> animation_;      //!< アニメーション
    std::shared_ptr<Texture>   tex_diffuse_;    //!< ディフューズテクスチャ
    std::shared_ptr<Texture>   tex_specular_;   //!< スペキュラーテクスチャ
    std::shared_ptr<Texture>   tex_albedo_;     //!< アルベドテクスチャ
    std::shared_ptr<Texture>   tex_normal_;     //!< 法線テクスチャ
    std::shared_ptr<Texture>   tex_ao_;         //!< AmbientOcclusionテクスチャ

    std::shared_ptr<Model>   model_knife_;         //!< 3Dモデル
    std::shared_ptr<Texture> tex_kn0_albedo_;      //!< アルベドテクスチャ
    std::shared_ptr<Texture> tex_kn0_normal_;      //!< 法線テクスチャ
    std::shared_ptr<Texture> tex_kn0_roughness_;   //!< ラフネステクスチャ
    std::shared_ptr<Texture> tex_kn0_metalness_;   //!< メタルネステクスチャ

    std::shared_ptr<Texture> tex_kn1_albedo_;      //!< アルベドテクスチャ
    std::shared_ptr<Texture> tex_kn1_normal_;      //!< 法線テクスチャ
    std::shared_ptr<Texture> tex_kn1_roughness_;   //!< ラフネステクスチャ
    std::shared_ptr<Texture> tex_kn1_metalness_;   //!< メタルネステクスチャ

    std::shared_ptr<Texture> tex_crosschairs_;   //!< 照準 reticle
};
