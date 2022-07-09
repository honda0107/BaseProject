//---------------------------------------------------------------------------
//! @file   ScenePhysicsLayer.h
//! @brief  物理 衝突レイヤー分けサンプル
//---------------------------------------------------------------------------
#pragma once

#include <System/Scene.h>

namespace physics
{
class Character;
class RigidBody;
}   // namespace physics

//===========================================================================
//! 物理シミュレーションサンプルシーン
//===========================================================================
class ScenePhysicsLayer final : public Scene::Base
{
public:
    BP_CLASS_TYPE(ScenePhysicsLayer, Scene::Base);

    //! シーン名称
    virtual std::string Name() override { return u8"[Physics] 衝突レイヤー分けサンプル"; }

    virtual bool Init() override;                // 初期化
    virtual void Update(float delta) override;   // 更新
    virtual void Draw() override;                // 描画
    virtual void Exit() override;                // 終了
    virtual void GUI() override;                 // GUI表示

private:
    //  物理シミュレーションをリセット
    void ResetPhysics();

private:
    std::shared_ptr<Model> model_box1_;     //!< ボックス
    std::shared_ptr<Model> model_box2_;     //!< コンテナボックス
    std::shared_ptr<Model> model_barrel_;   //!< ドラム缶
    std::shared_ptr<Model> model_cone_;     //!< 三角コーン

    std::shared_ptr<physics::Character> character_;     //!< キャラクターコントローラー
    std::shared_ptr<physics::RigidBody> body_floor_;    //!< 床
    std::shared_ptr<physics::RigidBody> rigid_body0_;   //!< 剛体
    std::shared_ptr<physics::RigidBody> rigid_body1_;   //!< 剛体
    std::shared_ptr<physics::RigidBody> rigid_body2_;   //!< 剛体
    std::shared_ptr<physics::RigidBody> rigid_body3_;   //!< 剛体
};
