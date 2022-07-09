//---------------------------------------------------------------------------
//! @file   SceneShader.h
//! @brief  シェーダーサンプルシーン
//---------------------------------------------------------------------------
#pragma once

#include <System/Scene.h>

class ComponentModel;
class ComponentFilterFade;

//===========================================================================
//! シェーダーシーン
//===========================================================================
class SceneShader final : public Scene::Base
{
public:
    BP_CLASS_TYPE(SceneShader, Scene::Base);

    //! シーン名称
    std::string Name() override { return u8"シェーダーサンプル"; }

    bool Init() override;                //!< 初期化
    void Update(float delta) override;   //!< 更新
    void Draw() override;                //!< 描画
    void Exit() override;                //!< 終了
    void GUI() override;                 //!< GUI表示

private:
    std::shared_ptr<ComponentModel>      model_;         //!< モデルコンポーネント
    std::shared_ptr<ComponentFilterFade> filter_fade_;   //!< フェードコンポーネント

    std::shared_ptr<Texture>  texture_;        //!< テクスチャ
    std::shared_ptr<ShaderVs> shader_vs_;      //!< 頂点シェーダー
    std::shared_ptr<ShaderPs> shader_ps_;      //!< ピクセルシェーダー(2D)
    std::shared_ptr<ShaderPs> shader_ps_3d_;   //!< ピクセルシェーダー(3D)
};
