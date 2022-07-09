#include "SceneTestCollision.h"
#include <System/Component/ComponentModel.h>
#include <System/Component/ComponentCamera.h>
#include <System/Component/ComponentCollisionSphere.h>
#include <System/Component/ComponentCollisionCapsule.h>

BP_CLASS_IMPL(SceneTestCollision, u8"TestCollision");

// Objectを継承してOnHitをオーバーライドすることで
// 当たった時に爆発させたり、ダメージを与えたりできる
class ObjectMouse : public Object
{
public:
    bool Init()
    {
        __super::Init();

        SetName("Mouse1");
        SetTranslate({-1, 0, 0});

        // モデルコンポーネント(0.05倍)
        AddComponent<ComponentModel>("data/Sample/Player/model.mv1")->SetScaleAxisXYZ({0.05f});

        // コリジョン(カプセル)
        AddComponent<ComponentCollisionCapsule>()->SetTranslate({0, 0, 0});

        // コリジョン(Sphere) 2つめの当たり(当たりのコンポーネントは複数持てます)
        auto col = AddComponent<ComponentCollisionSphere>()->SetTranslate({0, 0, 0})->SetRadius(0.5);
        // モデルに10番のノードにアタッチします
        col->AttachToModel(10);

        return true;
    }

    void OnHit([[maybe_unused]] const ComponentCollision::HitInfo& hitInfo) override
    {
        // 次のownerのオブジェクトと当たった!
        auto owner = hitInfo.hit_collision_->GetOwnerPtr();
        printfDx("当たった : %s", owner->GetName().data());

        // 当たりで移動させる(これが無ければめり込みます)
        __super::OnHit(hitInfo);
    }
};

bool SceneTestCollision::Init()
{
    //----------------------------------------------------------------------------------
    // CAMERA
    //----------------------------------------------------------------------------------
    auto cam = Scene::CreateObject<Object>()->SetName("Camera")->AddComponent<ComponentCamera>();

    //----------------------------------------------------------------------------------
    // OBJECT1
    //----------------------------------------------------------------------------------
    // オブジェクト作成
    Scene::CreateObject<ObjectMouse>()->SetTranslate({-3, 0, 0});

    Scene::CreateObject<ObjectMouse>()->SetTranslate({3, 0, 0});

    //----------------------------------------------------------------------------------
    // OBJECT2
    //----------------------------------------------------------------------------------
    {
        // TestObj / 位置(3,0,0)
        auto obj = Scene::CreateObject<Object>()->SetName("TestObj")->SetTranslate({3, 0, 0});

        // TestObj、モデルコンポーネント(0.08倍)
        auto comp = obj->AddComponent<ComponentModel>("data/Sample/Player/model.mv1")->SetScaleAxisXYZ({0.08f});

        // コリジョン(カプセル)
        obj->AddComponent<ComponentCollisionCapsule>()->SetTranslate({0, 0, 0})->SetHeight(6.0)->SetRadius(1.5);
    }

    //----------------------------------------------------------------------------------
    // OBJECT3
    //----------------------------------------------------------------------------------
    {
        // Mouse1 / 位置(3,0,0)
        auto obj = Scene::CreateObject<Object>()->SetName("Sphere")->SetTranslate({0, -2, 0});

        // コリジョン(カプセル)
        obj->AddComponent<ComponentCollisionSphere>()->SetTranslate({0, 0, 0});
    }
    return true;
}

void SceneTestCollision::Update([[maybe_unused]] float delta)
{
}

void SceneTestCollision::Draw()
{
    // とりあえずTitleという文字を表示しておく
    DrawFormatString(100, 50, GetColor(255, 255, 255), "Collision");
}

void SceneTestCollision::Exit()
{
    // タイトル終了時に行いたいことは今はない
}

void SceneTestCollision::GUI()
{
}
