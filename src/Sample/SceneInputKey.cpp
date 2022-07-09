//---------------------------------------------------------------------------
//! @file   SceneInputKey.cpp
//! @brief  キー入力とEffekseer
//---------------------------------------------------------------------------
#include "SceneInputKey.h"
#include <System/Component/ComponentModel.h>
#include <System/Component/ComponentCamera.h>
#include <System/Graphics/Animation.h>

BP_CLASS_IMPL(SceneInputKey, u8"キー入力とEffekseer")

namespace
{
class Camera : public Object
{
public:
    bool Init() override
    {
        __super::Init();

        SetName("Camera");
        camera_ = AddComponent<ComponentCamera>();

        SetTranslate({0, 2.5, 1.0});

        return true;
    }

private:
    std::shared_ptr<ComponentCamera> camera_;
    std::shared_ptr<Object>          target_;
};

class Player : public Object
{
    //-------------------------------------------------------
    //! アニメーション定義
    //-------------------------------------------------------
    // {アニメーション名, ファイル名, ファイル内のアニメーション番号, 再生速度}
    static const inline Animation::Desc desc_[] = {
        { "idle",  "data/Sample/Player/Anim/Idle2.mv1", 0,  1.0f},
        { "jump",  "data/Sample/Player/Anim/Jump2.mv1", 0,  1.0f},
        { "walk",   "data/Sample/Player/Anim/Walk.mv1", 1,  1.0f},
        {"walk2",  "data/Sample/Player/Anim/Walk2.mv1", 1, 0.75f},

        {"dance", "data/Sample/Player/Anim/Dance4.mv1", 0,  1.0f},
    };

public:
    bool Init() override
    {
        __super::Init();

        SetName(u8"プレイヤー");
        // モデルの追加
        model_ = std::make_shared<Model>("data/Sample/Player/model.mv1");
        // モデルを縮小表示
        matrix mat_world = mul(matrix::scale(0.03f), matrix::translate(position_));
        model_->setWorldMatrix(mat_world);

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
        animation_->play("idle", true);
        anim_status_      = 0;
        anim_status_prev_ = anim_status_;

        //-------------------------------------------------------
        // エフェクト
        //-------------------------------------------------------
        effect_handle_      = LoadEffekseerEffect("data/Sample/Effects/MagicFire1.efkefc");
        effect_play_handle_ = PlayEffekseer3DEffect(effect_handle_);
        // ひとまずプレイヤー座標にエフェクト出す
        SetPosPlayingEffekseer3DEffect(effect_play_handle_, position_.x, position_.y, position_.z);

        // 初期済にする。GUIは非表示。
        SetStatus(StatusBit::Initialized, true);
        return true;
    }

    //  更新
    virtual void Update(float delta) override
    {
        bool is_loop = true;   // アニメーションがループするかどうかフラグ

        //-------------------------------------------------------
        // 移動処理
        //-------------------------------------------------------
        {
            f32    per  = 0.05f;
            float3 move = float3(0.f, 0.f, 0.f);
            // キー入力で移動
            //  後進
            if(IsKeyRepeat(KEY_INPUT_W)) {
                key_release_frame_ = 0;
                //animation_->play("walk2", true);
                anim_status_ = 3;
                per          = 0.025f;
                move.z += 1.f;
            }
            //  前進
            if(IsKeyRepeat(KEY_INPUT_S)) {
                key_release_frame_ = 0;
                //animation_->play("walk1", true);
                anim_status_ = 2;
                move.z -= 1.f;
            }

            // ベクトルを使用した移動
            if(dot(move, move).x != 0.f) {
                move = normalize(move);
            }
            position_ += move * per;
            // 移動ベクトルを行列に乗算
            matrix mat_world = mul(matrix::scale(0.03f), matrix::translate(position_));
            // モデルにセット
            model_->setWorldMatrix(mat_world);
        }
        // ジャンプ処理
        if(IsKeyOn(KEY_INPUT_SPACE)) {
            key_release_frame_ = 0;
            //animation_->play("jump", false);
            anim_status_ = 1;
            is_jump_     = true;
            is_loop      = false;
        }

        if(IsKeyRelease(KEY_INPUT_W) && IsKeyRelease(KEY_INPUT_S) && is_jump_ == false) {
            // Wキー・Sキーが押されていなくて、かつジャンプしてなかったら
            // 押されてないカウンター増やす
            key_release_frame_ += delta;
        }

        // キーが押されていない状態が0.05より多ければ待機
        if(key_release_frame_ > 0.05f) {
            anim_status_ = 0;
        }
        // キーが押されていない状態が15より多ければダンス
        if(key_release_frame_ > 15.f) {
            anim_status_ = 4;
        }

        // ジャンプ中にアニメーションの再生が終了した
        if(animation_->isPlaying() == false && is_jump_ == true) {
            is_jump_           = false;
            anim_status_       = 0;
            key_release_frame_ = 0;
        }

        // ひとつ前のアニメーションと違っていたら、
        // そのアニメーションを再生する
        if(anim_status_ != anim_status_prev_) {
            anim_status_prev_ = anim_status_;
            animation_->play(desc_[anim_status_].name_, is_loop);
        }

        // アニメーション再生時間を進める
        animation_->update(delta);

        //-------------------------------------------------------
        // エフェクト
        //-------------------------------------------------------
        {
            // エフェクトをアニメーションのフレームにアタッチする
            auto   attach_frame  = MV1SearchFrame(*model_, "mixamorig:RightHandIndex1");
            auto   right_arm_mat = MV1GetFrameLocalWorldMatrix(*model_, attach_frame);
            float3 effect_pos    = mul(float4(position_, 1.0f), cast(right_arm_mat)).xyz;

            // エフェクトのループ再生
            if(IsEffekseer3DEffectPlaying(effect_play_handle_)) {
                // 再生終了でここに来る
                effect_play_handle_ = PlayEffekseer3DEffect(effect_handle_);
            }

            SetPosPlayingEffekseer3DEffect(effect_play_handle_, effect_pos.x, effect_pos.y, effect_pos.z);
        }
    }

    //! 描画
    void Draw() override { model_->render(); }

    //! 終了
    void Exit() override
    {
        __super::Exit();
        // Effekseerモデルの削除
        DeleteEffekseerEffect(effect_handle_);
    }

private:
    std::shared_ptr<Model>     model_;                      //!< 3Dモデル
    std::shared_ptr<Animation> animation_;                  //!< アニメーション
    f32                        key_release_frame_  = 0.f;   //!< キーが離されている間のフレーム数
    float3                     position_           = float3(0.f, 0.f, 0.f);   //!< 位置
    u32                        anim_status_        = 0;                       //!< 現在のアニメーション
    u32                        anim_status_prev_   = 0;                       //!< ひとつ前のアニメーション
    bool                       is_jump_            = false;                   //!< ジャンプ中
    int                        effect_handle_      = -1;                      //!< エフェクト
    int                        effect_play_handle_ = -1;                      //!< エフェクト 再生用
};

};   // namespace

//---------------------------------------------------------------------------
//! 初期化
//---------------------------------------------------------------------------
bool SceneInputKey::Init()
{
    Scene::CreateObject<Camera>();
    Scene::CreateObject<Player>();

    return true;
}

//---------------------------------------------------------------------------
//! 更新
//! @param  [in]    delta   経過時間
//---------------------------------------------------------------------------
void SceneInputKey::Update([[maybe_unused]] float delta)
{
}

//---------------------------------------------------------------------------
//! 描画
//---------------------------------------------------------------------------
void SceneInputKey::Draw()
{
}

//---------------------------------------------------------------------------
//! 終了
//---------------------------------------------------------------------------
void SceneInputKey::Exit()
{
}

//---------------------------------------------------------------------------
//! GUI表示
//---------------------------------------------------------------------------
void SceneInputKey::GUI()
{
}
