//---------------------------------------------------------------------------
//! @file   SceneRender.h
//! @brief  サンプルシーン
//---------------------------------------------------------------------------
#pragma once

#include <System/Scene.h>

class ComponentModel;
class ComponentCamera;

//===========================================================================
//! サンプルシーン
//===========================================================================
class SceneRender final : public Scene::Base
{
public:
    BP_CLASS_TYPE(SceneRender, Scene::Base)

    //! シーン名称
    std::string Name() override { return u8"サンプル"; }

    bool Init() override;                //!< 初期化
    void Update(float delta) override;   //!< 更新
    void Draw() override;                //!< 描画
    void Exit() override;                //!< 終了
    void GUI() override;                 //!< GUI表示

private:
    // int    model_    = -1;                         //!< [Dxlib] MV1モデルハンドル

    std::shared_ptr<ComponentModel>  model_;    //!<  モデルコンポーネント
    std::shared_ptr<ComponentCamera> camera_;   //!<  カメラコンポーネント

    float3 position_ = float3(0.0f, 0.0f, 0.0f);   //!< 位置
};
