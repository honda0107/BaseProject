#include "SceneTestWrong.h"
#include <System/Component/ComponentModel.h>

//namespace WRONG
//{

// ここにMenu設定を用意します
BP_CLASS_IMPL(SceneTestWrong, u8"SceneTest / Component間違い使用");

#define USE_WRONG_SOURCE   // 有効にすると間違いのソースを使います

#ifdef USE_WRONG_SOURCE

//! @brief ネズミクラス(間違いソース)
class TestMouse : public Object
{
public:
    // これはやってはいけない!
    // 解放できなくなったりメモリ解放もれの原因になります。
    // Objectに ComponentのSharedポインタはもってはいけません
    ComponentModelPtr model_ = nullptr;

    TestMouse() {}

    virtual ~TestMouse() {}

    bool Init() override
    {
        __super::Init();

        SetName("Mouse");

        // ここが間違い!!! Objectに Componentはそのままとらえてはいけない!!!
        // モデルが解放できず、持ち続けてクラッシュしてしまう
        model_ = AddComponent<ComponentModel>();

        model_->Load("data/Sample/Player/model.mv1");
        model_->SetAnimation({
            {"idle", "data/Sample/Player/Anim/Idle.mv1", 1, 1.0f}, // idle
            {"jump", "data/Sample/Player/Anim/Jump.mv1", 1, 1.0f}  // jump
        });

        return true;
    }

    void Draw() override { printfDx("DX!"); }
#if 0
	// これを入れることでもなおります (ただshared_ptrなのに解放しないといけないのはちょっと…)
	void Exit() override
	{
		model_ = nullptr;
	}
#endif
};
#else
// 正しいソース
class TestMouse : public Object
{
public:
    // SceneTestComponentのようにComponentは必要な時に取得するのが良いが、
    // とらえたい場合は弱参照にしてとらえる必要がある
    ComponentModelWeakPtr weak_model_{};

    bool Init() override
    {
        __super::Init();

        SetName("Mouse");

        // 弱参照としてとらえる
        weak_model_ = AddComponent<ComponentModel>();
        if(auto model = weak_model_.lock()) {
            // 弱参照がある場合のみ正式な変数として同じように更新する
            model->Load("data/Sample/Player/model.mv1");
            model->SetAnimation({
                {"idle", "data/Sample/Player/Anim/Idle.mv1", 1, 1.0f}, // idle
                {"jump", "data/Sample/Player/Anim/Jump.mv1", 1, 1.0f}  // jump
            });
        }
        return true;
    }
};

#endif

//! @brief シーン初期化関数を継承します
//! @return シーン初期化が終わったらtrueを返します
bool SceneTestWrong::Init()
{
    Scene::CreateObject<Object>("Camera")->AddComponent<ComponentCamera>()->SetPositionAndTarget({0, 40, 200},
                                                                                                 {0, 0, 0});

    // Componentとは、個別の特殊な単体の能力持っている構造です。
    // Component単体では生成することはできません。あくまでObjectの機能として動作します
    // 作成すると自動的に以下の関数が呼ばれます
    // Init()   <= 最初に一度呼ばれます
    // Update() <= ディスプレイリフレッシュレートに合わせて呼び出されます  例 60Hz : 1秒間に60回呼び出されます
    // Draw()   <= Updateの後に呼び出されます。描画するのに適した関数です
    // Exit()   <= 終了時に呼ばれる関数です

    // Mouseという名前のオブジェクトを作成します
    Scene::CreateObject<TestMouse>();

    return true;
}

//! @brief シーン更新関数。ディスプレイリフレッシュレートに合わせて実行されます
//! @param delta 1秒をベースとした1フレームの数値
//! @detial deltaは、リフレッシュレートが違うと速度が変わってしまう部分を吸収するためにある
void SceneTestWrong::Update([[maybe_unused]] float delta)
{
    // 上記で作成した MouseというObjectを取得します
    auto obj = Scene::GetObjectPtr<TestMouse>("Mouse");

    obj->AddTranslate({0.1f, 0, 0});

    auto new_rotate = obj->GetRotationAxisXYZ() + float3{0, 1, 0};
    obj->SetRotationAxisXYZ(new_rotate);

    // スペースを押すとジャンプする
    if(IsKeyOn(KEY_INPUT_SPACE)) {
#ifdef USE_WRONG_SOURCE
        // モデルのコンポーネントを取得してアクションをジャンプにする
        obj->model_->PlayAnimation("jump");
#else
        // モデルのコンポーネントを取得してアクションをジャンプにする
        if(auto model = obj->weak_model_.lock())
            model->PlayAnimation("jump");
#endif
    }
}

//}	// namespace WRONG
