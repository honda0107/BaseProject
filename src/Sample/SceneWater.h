//---------//---------------------------------------------------------------------------
//---------//---------------------------------------------------------------------------
//! @file   SceneWater.h
//! @brief  水面サンプルシーン
//---------------------------------------------------------------------------
#pragma once

#include <System/Scene.h>

class Model;

//===========================================================================
//! 水面サンプルシーン
//===========================================================================
class SceneWater final : public Scene::Base
{
public:
    BP_CLASS_TYPE(SceneWater, Scene::Base);

    //----------------------------------------------------------
    //! @name   シーン関数
    //----------------------------------------------------------
    //@{

    //! シーン名称を取得します
    virtual std::string Name() override { return u8"水面サンプル"; }

    virtual bool Init() override;                //!< 初期化
    virtual void Update(float delta) override;   //!< 更新
    virtual void Draw() override;                //!< 描画
    virtual void Exit() override;                //!< 終了
    virtual void GUI() override;                 //!< GUI表示

    //@}

private:
    std::shared_ptr<Model> model_;   //!< 3Dモデル
};
