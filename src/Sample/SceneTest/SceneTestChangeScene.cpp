#include "SceneTestChangeScene.h"
#include <System/Component/ComponentModel.h>
#include <System/Component/ComponentFilterFade.h>

// ここにMenu設定を用意します
BP_CLASS_IMPL(SceneTestChangeScene, u8"SceneTest / シーン切り替えサンプル");

namespace
{
void FadeIn()
{
    Scene::CreateObject<Object>()               // フェード用オブジェクト
        ->SetName("FadeIn")                     // 名前設定
        ->AddComponent<ComponentFilterFade>()   // フェードコンポーネント
        ->StartFadeIn();                        // フェードインスタート
}

bool WaitFadeIn()
{
    if(auto obj = Scene::GetObjectPtr<Object>("FadeIn")) {
        auto fade = obj->GetComponent<ComponentFilterFade>();
        if(fade->IsFadeIn()) {
            if(!fade->IsFinishFade())
                return true;

            Scene::ReleaseObject("FadeIn");
            return false;
        }
    }

    return false;
}

void FadeOut()
{
    Scene::CreateObject<Object>()               // フェード用オブジェクト
        ->SetName("FadeOut")                    // 名前設定
        ->AddComponent<ComponentFilterFade>()   // フェードコンポーネント
        ->StartFadeOut();                       // フェードインスタート
}

bool WaitFadeOut()
{
    if(auto obj = Scene::GetObjectPtr<Object>("FadeOut")) {
        auto fade = obj->GetComponent<ComponentFilterFade>();
        if(fade->IsFadeOut()) {
            if(!fade->IsFinishFade()) {
                return true;
            }
            Scene::ReleaseObject("FadeOut");
            return false;
        }
    }

    return true;
}

}   // namespace

bool SceneTestChangeScene::Init()
{
    auto obj = Scene::CreateObject<Object>()->SetName("Mouse");

    auto model = obj->AddComponent<ComponentModel>();
    model->Load("data/Sample/Player/model.mv1");
    model->SetAnimation({
        {"idle", "data/Sample/Player/Anim/Idle.mv1", 1, 1.0f}, // idle
        {"jump", "data/Sample/Player/Anim/Jump.mv1", 1, 1.0f}  // jump
    });

    // カメラ
    Scene::CreateObject<Object>()                          // カメラオブジェクト
        ->SetName("Camera")                                // 名前設定
        ->AddComponent<ComponentCamera>()                  // カメラコンポーネント
        ->SetPositionAndTarget({0, 35, 60}, {0, 20, 0});   // ポジションと注視点

    FadeIn();

    return true;
}

void SceneTestChangeScene::Update([[maybe_unused]] float delta)
{
    if(WaitFadeIn())
        return;

    // 上記で作成した MouseというObjectを取得します
    auto obj = Scene::GetObjectPtr<Object>("Mouse");

    auto new_rotate = obj->GetRotationAxisXYZ() + float3{0, 1, 0};
    obj->SetRotationAxisXYZ(new_rotate);

    if(auto model = obj->GetComponent<ComponentModel>()) {
        if(model->GetPlayAnimationName() != "idle")
            model->PlayAnimation("idle", true);
    }

    // スペースを押すとシーンを切り替える
    if(IsKeyOn(KEY_INPUT_SPACE)) {
        FadeOut();
    }

    if(!WaitFadeOut()) {
        // シーンを切り替えます
        Scene::Change(Scene::GetScene<SceneTestChangeScene2>());
    }
}

bool SceneTestChangeScene2::Init()
{
    auto obj = Scene::CreateObject<Object>()->SetName("Mouse");

    auto model = obj->AddComponent<ComponentModel>();
    model->Load("data/Sample/Player/model.mv1");
    model->SetAnimation({
        {"idle", "data/Sample/Player/Anim/Idle.mv1", 1, 1.0f}, // idle
        {"jump", "data/Sample/Player/Anim/Jump.mv1", 1, 1.0f}  // jump
    });

    auto obj2 = Scene::CreateObject<Object>()->SetName("Mouse");
    obj2->AddTranslate({10, 0, 0})->SetScaleAxisXYZ({2.0f})->SetRotationAxisXYZ({0, 180, 0});
    model = obj2->AddComponent<ComponentModel>();
    model->Load("data/Sample/Player/model.mv1");
    model->SetAnimation({
        {"idle", "data/Sample/Player/Anim/Idle.mv1", 1, 1.0f}, // idle
        {"jump", "data/Sample/Player/Anim/Jump.mv1", 1, 1.0f}  // jump
    });

    // カメラ
    Scene::CreateObject<Object>()                          // カメラオブジェクト
        ->SetName("Camera")                                // 名前設定
        ->AddComponent<ComponentCamera>()                  // カメラコンポーネント
        ->SetPositionAndTarget({0, 35, 60}, {0, 20, 0});   // ポジションと注視点

    FadeIn();

    return true;
}

void SceneTestChangeScene2::Update([[maybe_unused]] float delta)
{
    if(WaitFadeIn())
        return;

    // 上記で作成した MouseというObjectを取得します
    auto obj = Scene::GetObjectPtr<Object>("Mouse");

    auto new_rotate = obj->GetRotationAxisXYZ() + float3{0, -1, 0};
    obj->SetRotationAxisXYZ(new_rotate);

    if(auto model = obj->GetComponent<ComponentModel>()) {
        if(model->GetPlayAnimationName() != "jump")
            model->PlayAnimation("jump");
    }

    // スペースを押すとシーンを切り替える
    if(IsKeyOn(KEY_INPUT_SPACE))
        FadeOut();

    // フェードアウトを待ちます
    if(!WaitFadeOut()) {
        // シーンを切り替えます
        Scene::Change(Scene::GetScene<SceneTestChangeScene>());
    }
}
