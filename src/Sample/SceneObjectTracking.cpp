#include "SceneObjectTracking.h"
#include <System/Component/ComponentModel.h>
#include <System/Component/ComponentTargetTracking.h>

// ここにMenu設定を用意します
BP_CLASS_IMPL(SceneObjectTracking, u8"Object Tracking 使用サンプル");

//! @brief シーン初期化関数を継承します
//! @return シーン初期化が終わったらtrueを返します
bool SceneObjectTracking::Init()
{
    auto obj = Scene::CreateObject<Object>()->SetName("Object");
    obj->SetTranslate({0, 10, 10});

    // カメラ
    Scene::CreateObject<Object>()                          // カメラオブジェクト
        ->SetName("Camera")                                // 名前設定
        ->AddComponent<ComponentCamera>()                  // カメラコンポーネント
        ->SetPositionAndTarget({0, 35, 80}, {0, 20, 0});   // ポジションと注視点

    auto mouse = Scene::CreateObject<Object>()->SetName("Mouse");

    if(auto model = mouse->AddComponent<ComponentModel>()) {
        model->Load("data/Sample/Player/model.mv1");
        model->SetRotationAxisXYZ({0, 180, 0});
        model->SetAnimation({
            {"walk", "data/Sample/Player/Anim/Walk.mv1", 1, 1.0f}, // idle
            {"jump", "data/Sample/Player/Anim/Jump.mv1", 1, 1.0f}  // jump
        });
        model->PlayAnimation("walk", true);
    }
#if 1
    if(auto tracking = mouse->AddComponent<ComponentTargetTracking>()) {
        tracking->SetTargetObjectPtr(obj);
        tracking->SetTrackingNode("mixamorig:Neck");
        tracking->SetFrontVector({0, 0, 1});
    }
#endif

    return true;
}

//! @brief シーン更新関数。ディスプレイリフレッシュレートに合わせて実行されます
//! @param delta 1秒をベースとした1フレームの数値
//! @detial deltaは、リフレッシュレートが違うと速度が変わってしまう部分を吸収するためにある
void SceneObjectTracking::Update([[maybe_unused]] float delta)
{
}

void SceneObjectTracking::Exit()
{
}
