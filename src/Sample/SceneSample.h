//---------------------------------------------------------------------------
//! @file   SceneSample.h
//! @brief  サンプルシーン
//---------------------------------------------------------------------------
#pragma once

#include <System/Scene.h>

//===========================================================================
//! サンプルシーン
//===========================================================================
class SceneSample final : public Scene::Base
{
public:
    BP_CLASS_TYPE(SceneSample, Scene::Base);

    //! シーン名称
    std::string Name() override { return u8"サンプル"; }

    bool Init() override;                //!< 初期化
    void Update(float delta) override;   //!< 更新
    void Draw() override;                //!< 描画
    void Exit() override;                //!< 終了
    void GUI() override;                 //!< GUI表示

private:
    int model_ = -1;   //!< MV1モデルハンドル
};
