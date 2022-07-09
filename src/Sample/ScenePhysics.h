//---------------------------------------------------------------------------
//! @file   ScenePhysics.h
//! @brief  物理シミュレーションサンプルシーン
//---------------------------------------------------------------------------
#pragma once

#include <System/Scene.h>

namespace physics
{
class RigidBody;
}

//===========================================================================
//! 物理シミュレーションサンプルシーン
//===========================================================================
class ScenePhysics final : public Scene::Base
{
public:
    BP_CLASS_TYPE(ScenePhysics, Scene::Base);

    //! シーン名称
    virtual std::string Name() override { return u8"物理シミュレーションサンプル"; }

    virtual bool Init() override;                // 初期化
    virtual void Update(float delta) override;   // 更新
    virtual void Draw() override;                // 描画
    virtual void Exit() override;                // 終了
    virtual void GUI() override;                 // GUI表示

private:
    //  物理シミュレーションをリセット
    void ResetPhysics();

private:
    std::vector<std::shared_ptr<Model>> model_boxes1_;   //!< ボックス
    std::vector<std::shared_ptr<Model>> model_boxes2_;   //!< コンテナボックス
    std::vector<std::shared_ptr<Model>> model_barrel_;   //!< ドラム缶
    std::vector<std::shared_ptr<Model>> model_cone_;     //!< 三角コーン

    std::shared_ptr<physics::RigidBody>              body_floor_;     //!< 床
    std::vector<std::shared_ptr<physics::RigidBody>> rigid_bodies_;   //!< 剛体
};
