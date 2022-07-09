//---------------------------------------------------------------------------
//! @file   SceneCharacter.cpp
//! @brief  キャラクター操作サンプルシーン
//---------------------------------------------------------------------------
#include "SceneCharacter.h"

#include <System/Component/ComponentCamera.h>

#include <System/Physics/PhysicsEngine.h>
#include <System/Physics/PhysicsLayer.h>
#include <System/Physics/PhysicsCharacter.h>   // キャラクターコントローラー
#include <System/Physics/RigidBody.h>
#include <System/Physics/Shape.h>

#include "System/SystemMain.h"   // ShowGrid

BP_CLASS_IMPL(SceneCharacter, u8"キャラクター操作サンプル");

namespace
{

shape::Capsule shape_standing_  = shape::Capsule(0.8f, 0.3f);   //!< 立っている時
shape::Capsule shape_crouching_ = shape::Capsule(0.4f, 0.3f);   //!< しゃがんでいる時

}   // namespace

//---------------------------------------------------------------------------
//! 初期化
//---------------------------------------------------------------------------
bool SceneCharacter::Init()
{
    // グリッド表示をOFF
    ShowGrid(false);

    //-------------------------------------------------------
    // キャラクター
    //-------------------------------------------------------
    character_ = physics::createCharacter(physics::ObjectLayers::MOVING);

    // 形状を設定(必須)
    // 立っている時のシェイプ
    character_->setShape(shape_standing_, float3(0.0f, shape_standing_.half_height_ + shape_standing_.radius_, 0.0f));

    character_->setPosition(float3(0.0f, 5.0f, 0.0f));

    //-------------------------------------------------------
    // モデル
    //-------------------------------------------------------
    model_env_ = std::make_shared<Model>("data/Sample/空色町1.52/sorairo1.52.mv1");

    // 地形衝突情報を作成 (メッシュ剛体は必ず静的)
    body_env_ = physics::createRigidBody(shape::Mesh(model_env_.get(), 1.0f),   // メッシュ
                                         physics::ObjectLayers::NON_MOVING);    // 静的グループ

    //----------------------------------------------------------
    // カメラコンポーネント
    //----------------------------------------------------------
    auto obj = Scene::CreateObject<Object>();

    auto camera = obj->AddComponent<ComponentCamera>();
    camera->SetPerspective(60.0f);   // 画角
    camera->SetPositionAndTarget(float3(-15.0f, 12.0f, -15.0f), {0.0f, 5.0f, 0.0f});
    camera->SetCurrentCamera();

    return true;
}

//---------------------------------------------------------------------------
//! 更新
//! @param  [in]    delta   経過時間
//---------------------------------------------------------------------------
void SceneCharacter::Update([[maybe_unused]] float delta)
{
    auto* physicsEngine = physics::Engine::instance();

    // 更新前の座標を保存
    float3 old_position = character_->position();

    //----------------------------------------------------------
    // キャラクターを更新
    //----------------------------------------------------------
    character_->update(delta);

    //----------------------------------------------------------
    // 操作
    //----------------------------------------------------------
    float3 move_direction(0.0f, 0.0f, 0.0f);   // 移動ベクトル
    f32    character_speed_ = 1.0f;            // キャラクターの移動速度スケール

    if(IsKeyRepeat(KEY_INPUT_A)) {
        move_direction.z += 1.0f;
    }
    if(IsKeyRepeat(KEY_INPUT_D)) {
        move_direction.z -= 1.0f;
    }

    if(IsKeyRepeat(KEY_INPUT_W)) {
        move_direction.x += 1.0f;
    }
    if(IsKeyRepeat(KEY_INPUT_S)) {
        move_direction.x -= 1.0f;
    }

    // 移動方向を正規化
    if(dot(move_direction, move_direction).x != 0.0f) {
        move_direction = normalize(move_direction) * 10.0f;
    }

    // しゃがみ
    // 形状切り替え負荷があるため、毎回設定するのではなく押した瞬間・離した瞬間に切り替えるのが好ましい。
    // 常に切り替えると階段昇降がぎこちなくなる
    {
        static bool old_key = false;                      // 1フレーム前のキー
        bool        key     = IsKeyRepeat(KEY_INPUT_C);   // 現在のキー

        // 押した瞬間
        if(!old_key && key) {
            // しゃがんでいる時のシェイプ
            character_->setShape(shape_crouching_,
                                 float3(0.0f, shape_crouching_.half_height_ + shape_crouching_.radius_, 0.0f));
        }

        // 離した瞬間
        if(old_key && !key) {
            // 立っている時のシェイプ
            character_->setShape(shape_standing_,
                                 float3(0.0f, shape_standing_.half_height_ + shape_standing_.radius_, 0.0f));
        }

        // 次のフレームのために保存
        old_key = key;
    }

    // ジャンプ
    bool jump = false;
    if(IsKeyRepeat(KEY_INPUT_SPACE)) {
        jump = true;
    }

    float3 jump_direction = float3(0.0f, jump ? 8.0f : 0.0f, 0.0f);

    // 移動と背景衝突補正
    float3 new_velocity = character_->move(delta, move_direction, old_position, jump_direction);

    //------------------------------------------------------
    // 速度の反映
    //------------------------------------------------------

    // 重力
    new_velocity += physicsEngine->gravity() * delta;

    // プレイヤー操作
    new_velocity += move_direction * character_speed_;

    // 速度を更新
    character_->setLinearVelocity(new_velocity);
}

//---------------------------------------------------------------------------
//! 描画
//---------------------------------------------------------------------------
void SceneCharacter::Draw()
{
    // 青空のために画面クリア
    ClearColor(GetBackBuffer(), float4(0.3, 0.5f, 1.0f, 0.0f));

    //----------------------------------------------------------
    // モデル描画
    //----------------------------------------------------------
    model_env_->render();

    //----------------------------------------------------------
    // キャラクターを描画
    //----------------------------------------------------------
    auto* shape = character_->shape();
    auto  s     = static_cast<shape::Capsule*>(shape);   // カプセル形状

    float3 position = character_->position();
    DrawCapsule3D(cast(position + float3(0.0f, 1.f, 0.0f) * s->radius_),
                  cast(position + float3(0.0f, 1.f, 0.0f) * (s->radius_ + s->half_height_ * 2.0f)),
                  s->radius_,
                  8,
                  GetColor(0, 255, 0),
                  GetColor(0, 0, 0),
                  true);

    //----------------------------------------------------------
    // 操作説明を描画
    //----------------------------------------------------------
    {
        // 書式付き文字列の描画幅・高さ・行数を取得する
        const char* message = "[W/A/S/D] 移動   [C] しゃがみ   [SPACE] ジャンプ";
        s32         width;        // 幅
        s32         height;       // 高さ
        s32         line_count;   // 行数
        GetDrawFormatStringSize(&width, &height, &line_count, message);

        // センタリング
        s32 x = (WINDOW_W - width) / 2;
        s32 y = WINDOW_H - height - 32;

        DrawFormatString(x, y, GetColor(255, 255, 255), message);
    }
}

//---------------------------------------------------------------------------
//! 終了
//---------------------------------------------------------------------------
void SceneCharacter::Exit()
{
    // グリッド表示をON
    ShowGrid(true);
}

//---------------------------------------------------------------------------
//! GUI表示
//---------------------------------------------------------------------------
void SceneCharacter::GUI()
{
}
