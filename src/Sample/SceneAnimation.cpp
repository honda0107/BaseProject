//---------------------------------------------------------------------------
//! @file   SceneAnimation.cpp
//! @brief  アニメーションサンプルシーン
//---------------------------------------------------------------------------
#include "SceneAnimation.h"
#include <System/Component/ComponentModel.h>
#include <System/Component/ComponentCamera.h>

#include <System/Graphics/Animation.h>

BP_CLASS_IMPL(SceneAnimation, u8"アニメーションサンプル")

//===========================================================================
//! キャラクタークラス
//===========================================================================
class SceneAnimation::Character final : public Object
{
    //-------------------------------------------------------
    //! アニメーション定義
    // idle/jump/walk/walk2はアニメーションファイル内の
    // 0番目には何も入っていないため1番で登録
    //-------------------------------------------------------
    // {アニメーション名, ファイル名, ファイル内のアニメーション番号, 再生速度}
    static const inline Animation::Desc desc_[] = {
        {  "idle",   "data/Sample/Player/Anim/Idle.mv1", 1, 1.0f},
        {  "jump",   "data/Sample/Player/Anim/Jump.mv1", 1, 1.0f},
        {  "walk",   "data/Sample/Player/Anim/Walk.mv1", 1, 1.0f},
        { "walk2",  "data/Sample/Player/Anim/Walk2.mv1", 1, 1.0f},

        {"dance1", "data/Sample/Player/Anim/Dance1.mv1", 0, 1.0f},
        {"dance2", "data/Sample/Player/Anim/Dance2.mv1", 0, 1.0f},
        {"dance3", "data/Sample/Player/Anim/Dance3.mv1", 0, 1.0f},
        {"dance4", "data/Sample/Player/Anim/Dance4.mv1", 0, 1.0f},
        {"dance5", "data/Sample/Player/Anim/Dance5.mv1", 0, 1.0f},
    };

public:
    //  コンストラクタ
    Character() = default;

    //  デストラクタ
    virtual ~Character() {}

    //  初期化
    virtual bool Init() override
    {
        //-------------------------------------------------------
        // モデル
        //-------------------------------------------------------
        model_ = std::make_shared<Model>("data/Sample/Player/model.mv1");

        //-------------------------------------------------------
        // アニメーション
        //-------------------------------------------------------

        // アニメーション初期化
        animation_ = std::make_shared<Animation>(desc_, std::size(desc_));

        //  モデルにアニメーションを設定
        model_->bindAnimation(animation_.get());

        //-------------------------------------------------------
        // アニメーションを再生
        //-------------------------------------------------------
        f32 start_time = GetRand(100) * 0.2f;   // ランダムにアニメーション再生開始位置を設定
        animation_->play("dance4", true, 1.0f, start_time);   // is_loop = true

        // 初期済にする。GUIは非表示。
        SetStatus(StatusBit::Initialized, true);
        return true;
    }

    //  更新
    virtual void Update(float delta) override
    {
        timer_ += delta;

        // 5秒経過したらアニメーションを変更
        if(timer_ > 5.0f) {
            timer_ = 0.0f;

            // アニメーションをランダムで選択
            u32 animation_index = (rand() >> 9) % std::size(desc_);
            f32 start_time = GetRand(100) * 0.2f;   // ランダムにアニメーション再生開始位置を設定
            animation_->play(desc_[animation_index].name_, true, 1.0f, start_time);   // is_loop = true
        }

        // アニメーション再生時間を進める
        animation_->update(delta);
    }

    //! 描画
    void Draw() override { model_->render(); }

    //! 終了
    void Exit() override { __super::Exit(); }

    //! キャラクター番号を設定
    void setIndexAndPosition(u32 index)
    {
        index_ = index;

        //-------------------------------------------------------
        // キャラクターの立ち位置を設定
        //-------------------------------------------------------
        constexpr f32 INTERVAL = 1.5f;   // キャラクターの間隔

        // インデックスから-2, -1, 0, +1, +2 のオフセットを計算
        f32 offset = static_cast<f32>(index_ - 2);

        f32 x = offset * INTERVAL;
        f32 z = fabsf(offset) * 1.0f;

        matrix mat_world = mul(matrix::scale(0.01f), matrix::translate(float3(x, 0.0f, z)));
        model_->setWorldMatrix(mat_world);
    }

private:
    std::shared_ptr<Model>     model_;          //!< 3Dモデル
    std::shared_ptr<Animation> animation_;      //!< アニメーション
    s32                        index_ = 0;      //!< キャラクター番号
    f32                        timer_ = 0.0f;   //!< 経過時間
};

//===========================================================================
//  アニメーションサンプルシーン
//===========================================================================
namespace
{

static constexpr u32 CHARACTER_COUNT = 5;   //!< キャラクターの数

// キャラクター
std::vector<std::shared_ptr<SceneAnimation::Character>> characters_;

}   // namespace

//---------------------------------------------------------------------------
//! 初期化
//---------------------------------------------------------------------------
bool SceneAnimation::Init()
{
    //----------------------------------------------------------
    // キャラクターを複数作成
    //----------------------------------------------------------
    for(u32 i = 0; i < CHARACTER_COUNT; ++i) {
        // モデル
        auto o = Scene::CreateObject<Character>();
        o->SetName(u8"キャラクター");
        o->setIndexAndPosition(i);

        characters_.emplace_back(std::move(o));
    }

    //----------------------------------------------------------
    // カメラ
    //----------------------------------------------------------
    auto o = Scene::CreateObject<Object>();
    o->SetName(u8"カメラ");

    // カメラコンポーネントを追加
    auto camera = o->AddComponent<ComponentCamera>();

    constexpr f32 fov          = 60.0f;                 // 画角
    float3        eye_position = {1.5f, 1.0f, -2.0f};   // カメラの位置
    float3        look_at      = {0.5f, 0.5f, 1.0f};    // 注視点

    camera->SetPerspective(fov);
    camera->SetPositionAndTarget(eye_position, look_at);
    camera->SetCurrentCamera();

    return true;
}

//---------------------------------------------------------------------------
//! 更新
//! @param  [in]    delta   経過時間
//---------------------------------------------------------------------------
void SceneAnimation::Update([[maybe_unused]] float delta)
{
}

//---------------------------------------------------------------------------
//! 描画
//---------------------------------------------------------------------------
void SceneAnimation::Draw()
{
}

//---------------------------------------------------------------------------
//! 終了
//---------------------------------------------------------------------------
void SceneAnimation::Exit()
{
    // キャラクターを解放
    characters_.clear();
}

//---------------------------------------------------------------------------
//! GUI表示
//---------------------------------------------------------------------------
void SceneAnimation::GUI()
{
}
