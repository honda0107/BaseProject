#include "SceneTestObject.h"

// ここにMenu設定を用意します
BP_CLASS_IMPL(SceneTestObject, u8"SceneTest / Object使用方法サンプル");

namespace
{
//! このファイル内で使用できるオブジェクト
//! ただしこの確保方法はプログラムが存在する限りこのオブジェクトは存在するため注意が必要です
ObjectPtr static_object = nullptr;

}   // namespace

//! @brief シーン初期化関数を継承します
//! @return シーン初期化が終わったらtrueを返します
bool SceneTestObject::Init()
{
    // Objectとは、基本的に単なる箱です。機能(Component)と呼ばれるものをまとめておくことができます。
    // Objectには作成すると自動的に実行する以下の関数があります。
    // Init()   <= 最初に一度呼ばれます
    // Update() <= ディスプレイリフレッシュレートに合わせて呼び出されます  例 60Hz : 1秒間に60回呼び出されます
    // Draw()   <= Updateの後に呼び出されます。描画するのに適した関数です
    // Exit()   <= 終了時に呼ばれる関数です

    auto obj = Scene::CreateObject<Object>();
    // 上記の方法でオブジェクトを作成できます。
    // ここではauto変数にしてオブジェクトを確保しています。
    // Ctrlを2回押すとautoの型が見れます。

    // ただしこの方法ではobjは、Init()内でのみでしか使用できません。(スコープがInit内のため)
    // この方法は名前をつけたり、のちに説明する機能(Component)を追加する時に使用します

    obj->SetName("test");
    // 先ほど作成したオブジェクトに"test" という名前をつける

    static_object = Scene::CreateObject<Object>()->SetName("UseInFile");
    // 上記の方法では、このファイル内すべてでオブジェクトを使用できます
    // ->は、そのまま続けて書くことができます。
    // 今回オブジェクトの名前を UseInFile という名前にしています

    static_object->SetTranslate({10, 0, 0});
    // オブジェクトの位置を 10, 0, 0 に移動しています
    // GUI で確認できます

    Scene::CreateObject<Object>()->SetName("CanIUse?");
    // 上記のオブジェクトは作成するのみで、その後使用できないように見えます
    // ただこの状態でも使用は可能です。Update内で使用されています。

    return true;
}

//! @brief シーン更新関数。ディスプレイリフレッシュレートに合わせて実行されます
//! @param delta 1秒をベースとした1フレームの数値
//! @detial deltaは、リフレッシュレートが違うと速度が変わってしまう部分を吸収するためにある
void SceneTestObject::Update([[maybe_unused]] float delta)
{
    // [[maybe_unused]] という[[]] で囲まれたものは、コンパイラのワーニングを抑えるために使われています
    // 試しにこの[[maybe_unused]]をとってビルドすると「deltaが使用されてない」というようなエラーが出ます
    // ワーニングなので影響ないのですが、本来使っているはずの変数がミスで使用されていない場合がある時もあります
    // このようなミスが出ないようにするためWARNINGは必ずなくす規則にしている会社が多いため覚えておきましょう

    auto obj = Scene::GetObjectPtr<Object>("CanIUse?");
    // 上記は、Init()にて、使えない? と思ったオブジェクトの取得方法です
    // <Object>("CanIUse?")の部分を作った時と同じにすると取得可能です

    auto new_translate = obj->GetTranslate() + float3{0.1f, 0, 0};
    obj->SetTranslate(new_translate);
    // objの位置に 0.1f, 0, 0 を加えて新しい位置として設定する方法です
    // 以下も、X座標に1を加える別の方法です。以下のほうがわかりやすいかもしれません
    //
    // obj->AddTranslate( { 0.1f, 0, 0 } );
    // obj->Translate() += {0.1f,0,0};

    auto new_rotate = obj->GetRotationAxisXYZ() + float3{0, 1, 0};
    obj->SetRotationAxisXYZ(new_rotate);
}

void SceneTestObject::Exit()
{
    // 最終的にリークはしないが、このオブジェクトは別シーンに行っても残ってしまうので、
    // 利用しないのであればnullptrにして削除するほうが良いです
    // static_object = nullptr;
}
