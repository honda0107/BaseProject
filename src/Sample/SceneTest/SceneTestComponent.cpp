#include "SceneTestComponent.h"
#include <System/Component/ComponentModel.h>

// ここにMenu設定を用意します
BP_CLASS_IMPL(SceneTestComponent, u8"SceneTest / Component使用方法サンプル");

//! @brief シーン初期化関数を継承します
//! @return シーン初期化が終わったらtrueを返します
bool SceneTestComponent::Init()
{
    // Componentとは、個別の特殊な単体の能力持っている構造です。
    // Component単体では生成することはできません。あくまでObjectの機能として動作します
    // 作成すると自動的に以下の関数が呼ばれます
    // Init()   <= 最初に一度呼ばれます
    // Update() <= ディスプレイリフレッシュレートに合わせて呼び出されます  例 60Hz : 1秒間に60回呼び出されます
    // Draw()   <= Updateの後に呼び出されます。描画するのに適した関数です
    // Exit()   <= 終了時に呼ばれる関数です

    // Mouseという名前のオブジェクトを作成します
    auto obj = Scene::CreateObject<Object>()->SetName("Mouse");

    auto model = obj->AddComponent<ComponentModel>();
    // オブジェクトにモデル能力を持たせます
    // 感覚的には、通常以下のような仕組みを動的に行います。
    // class Object
    // {
    //     public:
    //         Model model;
    // };
    //
    // またこのComponentModelは、objの座標系を自動的に使用するため
    // objが移動すると、Componentモデルもそれに合わせて動作します

    model->Load("data/Sample/Player/model.mv1");
    model->SetAnimation({
        {"idle", "data/Sample/Player/Anim/Idle.mv1", 1, 1.0f}, // idle
        {"jump", "data/Sample/Player/Anim/Jump.mv1", 1, 1.0f}  // jump
    });

    return true;
}

//! @brief シーン更新関数。ディスプレイリフレッシュレートに合わせて実行されます
//! @param delta 1秒をベースとした1フレームの数値
//! @detial deltaは、リフレッシュレートが違うと速度が変わってしまう部分を吸収するためにある
void SceneTestComponent::Update([[maybe_unused]] float delta)
{
    // 上記で作成した MouseというObjectを取得します
    auto obj = Scene::GetObjectPtr<Object>("Mouse");

    obj->AddTranslate({0.1f, 0, 0});

    auto new_rotate = obj->GetRotationAxisXYZ() + float3{0, 1, 0};
    obj->SetRotationAxisXYZ(new_rotate);

    // スペースを押すとジャンプする
    if(IsKeyOn(KEY_INPUT_SPACE)) {
        // モデルのコンポーネントを取得してアクションをジャンプにする
        if(auto model = obj->GetComponent<ComponentModel>())
            model->PlayAnimation("jump");
    }
}
