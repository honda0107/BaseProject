//---------------------------------------------------------------------------
//! @file   SceneAnimation.h
//! @brief  アニメーションサンプルシーン
//---------------------------------------------------------------------------
#pragma once

#include <System/Scene.h>

//===========================================================================
//! アニメーションサンプルシーン
//===========================================================================
class SceneAnimation final : public Scene::Base
{
public:
    BP_CLASS_TYPE(SceneAnimation, Scene::Base)

    //----------------------------------------------------------
    //! @name   内部クラス
    //----------------------------------------------------------
    //@{

    class Character;   //!< キャラクタークラス

    //@}

    //! シーン名称
    std::string Name() override { return u8"アニメーションサンプル"; }

    bool Init() override;                //!< 初期化
    void Update(float delta) override;   //!< 更新
    void Draw() override;                //!< 描画
    void Exit() override;                //!< 終了
    void GUI() override;                 //!< GUI表示

private:
};
