#include "SceneTest1.h"
#include "SceneTest2.h"
#include <System/Component/ComponentModel.h>
#include <System/Component/ComponentCamera.h>
#include <System/Component/ComponentCollisionSphere.h>

BP_CLASS_IMPL(SceneTest1, u8"Test1");

namespace
{
static bool save_flag = false;

std::shared_ptr<SceneTest2> scene2 = nullptr;

}   // namespace

bool SceneTest1::Init()
{
    // セーブされていればロードする
    if(save_flag) {
        Load();
        return true;
    }

    // 次のシーンへ行っても状態を残したいときはこれをつける
    // AliveInAnotherScene();

    // 別の先に作成することができる※①
    auto scene = Scene::GetScene<SceneTest2>();

    // 自動的に名前がつく"object"となる
    // オブジェクトを作り、"test" という名前をつける
    auto obj = Scene::CreateObject<Object>()->SetName("test");

    // モデルコンポーネント
    auto comp = obj->AddComponent<ComponentModel>("data/Sample/Player/model.mv1");
    // モデルを 0.05倍にする (普通の書き方)
    comp->SetScaleAxisXYZ({0.05f});

    // コンポーネントもそのまま処理できます。
    // (0,2,0)の位置にCollisionSphereを移動させています
    obj->AddComponent<ComponentCollisionSphere>()->SetTranslate({0, 2, 0});

    // こちらはobjについているComponentTransformを直接操作しています
    obj->SetScaleAxisXYZ({0.5f})             //< スケールを0.5倍に
        ->SetTranslate({3, 0, 0})            //< 3,0,0 に移動
        ->SetRotationAxisXYZ({0, 180, 0});   //< 向きを180度回す

    // 自動的に別の名前がつく"object_1"となる (objectという名前はすでにないが一度付いたことになる)
    auto obj2 = Scene::CreateObject<Object>()->SetName("obj2");   //< オブジェクト作成
    obj2->AddComponent<ComponentCollisionSphere>();               //< 作成時につなげてしまうと
    obj2->AddComponent<ComponentCamera>();   //< コンポーネントが返されるのでつなげれません

    // ComponentTransformを使わないObjectサンプル
    Scene::CreateObject<Object>(true)->SetName("no-transform");

    // 次の追加は不十分な処理です。(シリアライズセーブされません)
    // シーンのInitSerialzeまたは、Object継承でInitSerialize()にてSetProcを行ってください
    obj2->SetProc("Test", []() { printfDx("Test!"); });

    return true;
}

void SceneTest1::InitSerialize()
{
    // 処理追加( この処理シリアライズはシーンスタートで必ず呼ばれる )
    if(auto obj2 = Scene::GetObjectPtr<Object>("obj2"))
        obj2->SetProc("Test", []() { printfDx("Test!"); });
}

void SceneTest1::Update([[maybe_unused]] float delta)
{
    // カメラの設定
    auto obj = Scene::GetObjectPtr<Object>("test");

    obj->AddRotationAxisXYZ({0, 1, 0});   //< 1度づつ回転 (時計回り)

    // スペースを押したら次に行く
    if(IsKeyOn(KEY_INPUT_SPACE)) {
        // std::shared_ptr<SceneTest2>() だと別シーンとなるため、
        // assertとなります。
        // ※①で先に作成した設定したシーンを設定しましょう
        Scene::Change(Scene::GetScene<SceneTest1>());

        // ステージ切り替えで状態をセーブする
        Save();
        save_flag = true;
    }
}

void SceneTest1::Draw()
{
    // とりあえずTitleという文字を表示しておく
    DrawFormatString(100,
                     50,
                     GetColor(255, 255, 255),
                     "Windos11 : %d, %d",
                     HelperLib::OS::IsWindows11(),
                     HelperLib::OS::IsWindows10OrGreater());
}

void SceneTest1::Exit()
{
    // タイトル終了時に行いたいことは今はない
}

void SceneTest1::GUI()
{
}
