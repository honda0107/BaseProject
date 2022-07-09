//---------------------------------------------------------------------------
//! @file   ScenePG15honda.h
//! @brief  PG15hondaシーン
//---------------------------------------------------------------------------
#pragma once

#include <System/Scene.h>

//===========================================================================
//! サンプルシーン
//===========================================================================
class ScenePG15honda final : public Scene::Base
{
public:
    BP_CLASS_TYPE(ScenePG15honda, Scene::Base);

    //! シーン名称
    std::string Name() override { return u8"PG15本田"; }

    bool Init() override;                //!< 初期化
    void Update(float delta) override;   //!< 更新
    void Draw() override;                //!< 描画
    void Exit() override;                //!< 終了
    void GUI() override;                 //!< GUI表示

private:
};
