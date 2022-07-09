//---------------------------------------------------------------------------
//! @file   PhysicsCharacter.cpp
//! @brief  キャラクターコントローラー
//---------------------------------------------------------------------------
#include <System/Physics/PhysicsEngine.h>
#include <System/Physics/PhysicsLayer.h>
#include <System/Physics/PhysicsCharacter.h>
#include <System/Physics/RigidBody.h>
#include <System/Physics/Shape.h>

//--------------------------------------------------------------
// JoltPhysicsインクルードファイル
//--------------------------------------------------------------
#include <Jolt/Jolt.h>

#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Collision/Shape/HeightFieldShape.h>
#include <Jolt/Physics/Collision/Shape/MeshShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/ConvexHullShape.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/TaperedCapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/CylinderShape.h>
#include <Jolt/Physics/Collision/Shape/TriangleShape.h>
#include <Jolt/Physics/Collision/Shape/StaticCompoundShape.h>
#include <Jolt/Physics/Collision/Shape/MutableCompoundShape.h>
#include <Jolt/Physics/Collision/Shape/RotatedTranslatedShape.h>
#include <Jolt/Physics/Collision/Shape/ScaledShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>

#include <Jolt/Physics/Character/CharacterVirtual.h>

namespace physics
{

//===========================================================================
//! キャラクターコントローラー (実装部)
//===========================================================================
class CharacterImpl
    : public Character
    , public JPH::CharacterContactListener
{
public:
    // コンストラクタ
    CharacterImpl(u16 layer);

    //  カスタムのコンタクトリスナー
    physics::Character::ContactListener* listener_ = nullptr;

    //----------------------------------------------------------
    //! @name   コンタクトリスナー
    //----------------------------------------------------------
    //@{

    //  コンタクトリスナーを設定
    virtual void setListener(physics::Character::ContactListener* listener) override;

    //  コンタクトリスナーを取得
    virtual physics::Character::ContactListener* listener() const override;

    //@}
    //----------------------------------------------------------
    //! @name   パラメーター
    //----------------------------------------------------------
    //@{

    //  速度を取得 (単位:m/s)
    virtual float3 linearVelocity() const override;

    //  速度を設定 (単位:m/s)
    virtual void setLinearVelocity(const float3& linear_velocity) override;

    //  位置を取得
    virtual float3 position() const override;

    //  位置を設定
    virtual void setPosition(const float3& p) override;

    //  回転姿勢を取得
    virtual quaternion rotation() const override;

    //  回転姿勢を設定
    virtual void setRotation(const quaternion& r) override;

    //  ワールド行列を取得
    virtual matrix worldMatrix() const override;

    //  重心の変換行列を取得
    virtual matrix centerOfMassTransform() const override;

    //  質量を設定 (単位:kg)
    virtual void setMass(f32 mass) override;

    //  キャラクターがまだ歩ける最大傾斜角度を設定する (単位:ラジアン)
    virtual void setMaxSlopeAngle(f32 max_slope_angle) override;

    //  キャラクターが他の物体を押すことができる最大の力を設定（単位:N）
    virtual void setMaxStrength(f32 max_strength) override;

    //  ジオメトリからどれだけ離れられるかの距離を取得
    virtual f32 characterPadding() const override;

    //@}

    //  更新
    //! 現在の速度に応じてキャラクターを移動させます。
    //! @param [in] delta_time  シミュレーションステップ時間
    virtual void update(f32 delta_time) override;

    //  移動
    //! @param  [in]    delta_time      シミュレーションステップ時間
    //! @param  [in]    move_vector     移動ベクトル
    //! @param  [in]    old_position    移動前の位置( update()実行前の座標が必要です )
    //! @param  [in]    jump_vector     ジャンプ方向ベクトル(ジャンプしない場合は0で渡します)
    //! @return 新しい補正後の移動速度
    //! 内部でwalkStairs()による階段歩行も行います
    virtual float3 move(f32           delta_time,
                        const float3& move_vector,
                        const float3& old_position,
                        const float3& jump_vector = float3(0.0f, 0.0f, 0.0f)) override;

    //  階段を歩けるかどうかを取得
    //! @details この関数はキャラクターが急すぎる斜面（垂直な壁など）に移動した場合にtrueを返します。
    //! 階段を上ろうとする場合はwalkStairsを呼び出すことになります。
    virtual bool canWalkStairs() const override;

    //  階段歩行
    //! @param  [in]    delta_time          シミュレーションのステップ時間
    //! @param  [in]    step_up             ステップアップする方向と距離（これが最大ステップ高に相当する）
    //! @param  [in]    step_forward        踏み台昇降の後に前に踏み出す方向と距離
    //! @param  [in]    step_forward_test   床を検査する方向と距離
    //! @param inStepDownExtra  ステップダウンする最後に加算される平行移動。上より下にステップダウンできるようにします。不要な場合は0に設定します。
    //! @return 階段の歩行が成功した場合はtrueが戻ります
    //!
    //! 引数 step_forward_test について
    //! 高い周波数で走行している場合 step_forward は非常に小さくなり、降りるときに階段の側面にぶつかる可能性があります。
    //! そのため、最大傾斜角度に違反する法線が発生する可能性があります。
    //! このような場合は上昇位置からこの距離を使って再度テストし、有効なスロープを見つけるかどうかを確認します。
    virtual bool walkStairs(f32           delta_time,
                            const float3& step_up,
                            const float3& step_forward,
                            const float3& step_forward_test,
                            const float3& step_down_extra = float3(0.0f, 0.0f, 0.0f)) override;

    //  衝突点を再検出
    //! この機能はキャラクターがテレポートした後に、新しい世界とのコンタクトを決定するために使用することができます。
    virtual void refleshContacts() override;

    //  キャラクターの形状を入れ替える（しゃがみ、構え方など）
    //! @param  [in]    shape           形状
    //! @param  [in]    shape_offset    形状位置をずらすオフセット
    //! @retval true    正常終了(形状の入れ替え成功)
    //! @retval false   形状の入れ替え後に衝突物があり、切り替えできなかった場合
    virtual bool setShape(const shape::Base& shape, const float3& shape_offset = float3(0.0f, 0.0f, 0.0f)) override;

    //  シェイプを取得
    virtual shape::Base* shape() const override;

    //  シェイプの移動オフセットを取得
    virtual float3 shapeOffset() const override;

    //@}
    //----------------------------------------------------------
    //! @name   接地判定関連
    //----------------------------------------------------------
    //@{

    //  現在の接地状態を取得
    virtual Character::GroundState groundState() const override;

    //  接地している足元の位置座標を取得
    virtual float3 groundPosition() const override;

    //  接地している足元の法線を取得
    virtual float3 groundNormal() const override;

    //  接地している足元の速度を取得
    virtual float3 groundVelocity() const override;

    //  キャラクターが立っているオブジェクトのボディIDを取得
    virtual u64 groundBodyID() const override;

    //@}
    //----------------------------------------------------------
    //! @name   JoltPhysicsから呼ばれるコンタクトリスナー
    //----------------------------------------------------------
    //@{

    //  キャラクターが指定されたボディと衝突可能かどうかをチェックします。
    //! @retval true    衝突可能
    //! @retval false   衝突しない
    virtual bool OnContactValidate([[maybe_unused]] const JPH::CharacterVirtual* character,
                                   const JPH::BodyID&                            body_id2,
                                   [[maybe_unused]] const JPH::SubShapeID&       sub_shape_id2) override;

    //  キャラクターがボディと衝突するたびに呼び出されます
    //! @retval true    接触がキャラクターを押すことができる
    //! @retval false   接触がキャラクターを押すことができない
    virtual void OnContactAdded([[maybe_unused]] const JPH::CharacterVirtual* character,
                                const JPH::BodyID&                            body_id2,
                                [[maybe_unused]] const JPH::SubShapeID&       sub_shape_id2,
                                JPH::Vec3Arg                                  contact_position,
                                JPH::Vec3Arg                                  contact_normal,
                                JPH::CharacterContactSettings&                settings) override;

    //@}

private:
    JPH::Ref<JPH::CharacterVirtual> jph_character_;                           //!< [JPH] Vitrualキャラクター
    JPH::ShapeRefC                  jph_shape_;                               //!< [JPH] 設定中のシェイプ
    u16                             layer_ = physics::ObjectLayers::MOVING;   //!< オブジェクトレイヤー

    std::unique_ptr<shape::Base> shape_;                                     //!< 形状
    float3                       shape_offset_ = float3(0.0f, 0.0f, 0.0f);   //!< シェイプの移動オフセット
};

//---------------------------------------------------------------------------
//! コンストラクタ
//---------------------------------------------------------------------------
CharacterImpl::CharacterImpl(u16 layer)
{
    layer_ = layer;

    //----------------------------------------------------------
    // キャラクターコントローラーを作成
    //----------------------------------------------------------
    JPH::CharacterVirtualSettings settings;
    {
        // 坂道として認識する最大傾斜角
        settings.mMaxSlopeAngle = 45.0f * DegToRad;

        // キャラクターが他の物体を押すことができる最大の力（N）
        settings.mMaxStrength = 2000.0f;

        // ジオメトリからどれだけ離れられるかの距離
        // これによってスイープができるだけ衝突しないようにして衝突コストを下げ、スタックするリスクを低減します。
        // ぷるぷる振動しない程度に設定
        settings.mCharacterPadding = 0.02f;

        // 予測接触で形状の外側をどこまでスキャンするか
        settings.mPredictiveContactDistance = 0.1f;

        // めり込み状態から回復速度(0.0f～1.0f)
        settings.mPenetrationRecoverySpeed = 1.0f;
    }

    // 作成
    // (この段階ではまだシェイプは設定されていない)
    jph_character_ = new JPH::CharacterVirtual(&settings,
                                               JPH::Vec3::sZero(),
                                               JPH::Quat::sIdentity(),
                                               physics::Engine::physicsSystem());

    // コンタクトリスナーを設定
    // (このクラス自身がコンタクトリスナーを継承しています)
    jph_character_->SetListener(this);
}

//---------------------------------------------------------------------------
//! コンタクトリスナーを設定
//---------------------------------------------------------------------------
void CharacterImpl::setListener(physics::Character::ContactListener* listener)
{
    listener_ = listener;
}

//---------------------------------------------------------------------------
//! コンタクトリスナーを取得
//---------------------------------------------------------------------------
physics::Character::ContactListener* CharacterImpl::listener() const
{
    return listener_;
}

//---------------------------------------------------------------------------
//! 速度を取得 (単位:m/s)
//---------------------------------------------------------------------------
float3 CharacterImpl::linearVelocity() const
{
    return castJPH(jph_character_->GetLinearVelocity());
}

//---------------------------------------------------------------------------
//! 速度を設定 (単位:m/s)
//---------------------------------------------------------------------------
void CharacterImpl::setLinearVelocity(const float3& linear_velocity)
{
    jph_character_->SetLinearVelocity(castJPH(linear_velocity));
}

//---------------------------------------------------------------------------
//! 位置を取得
//---------------------------------------------------------------------------
float3 CharacterImpl::position() const
{
    return castJPH(jph_character_->GetPosition());
}

//---------------------------------------------------------------------------
//! 位置を設定
//---------------------------------------------------------------------------
void CharacterImpl::setPosition(const float3& p)
{
    jph_character_->SetPosition(castJPH(p));
}

//---------------------------------------------------------------------------
//! 回転姿勢を取得
//---------------------------------------------------------------------------
quaternion CharacterImpl::rotation() const
{
    return castJPH(jph_character_->GetRotation());
}

//---------------------------------------------------------------------------
//! 回転姿勢を設定
//---------------------------------------------------------------------------
void CharacterImpl::setRotation(const quaternion& r)
{
    jph_character_->SetRotation(castJPH(r));
}

//---------------------------------------------------------------------------
//! ワールド行列を取得
//---------------------------------------------------------------------------
matrix CharacterImpl::worldMatrix() const
{
    return castJPH(jph_character_->GetWorldTransform());
}

//---------------------------------------------------------------------------
//! 重心の変換行列を取得
//---------------------------------------------------------------------------
matrix CharacterImpl::centerOfMassTransform() const
{
    return castJPH(jph_character_->GetCenterOfMassTransform());
}

//---------------------------------------------------------------------------
//! 質量を設定 (単位:kg)
//---------------------------------------------------------------------------
void CharacterImpl::setMass(f32 mass)
{
    jph_character_->SetMass(mass);
}

//---------------------------------------------------------------------------
//! キャラクターがまだ歩ける最大傾斜角度を設定する (単位:ラジアン)
//---------------------------------------------------------------------------
void CharacterImpl::setMaxSlopeAngle(f32 max_slope_angle)
{
    jph_character_->SetMaxSlopeAngle(max_slope_angle);
}

//---------------------------------------------------------------------------
//! キャラクターが他の物体を押すことができる最大の力を設定（単位:N）
//---------------------------------------------------------------------------
void CharacterImpl::setMaxStrength(f32 max_strength)
{
    jph_character_->SetMaxStrength(max_strength);
}

//---------------------------------------------------------------------------
//! ジオメトリからどれだけ離れられるかの距離を取得
//---------------------------------------------------------------------------
f32 CharacterImpl::characterPadding() const
{
    return jph_character_->GetCharacterPadding();
}

//---------------------------------------------------------------------------
//! 更新
//---------------------------------------------------------------------------
void CharacterImpl::update(f32 delta_time)
{
    auto* physics_system = physics::Engine::physicsSystem();

    jph_character_->Update(delta_time,
                           physics_system->GetGravity(),
                           physics_system->GetDefaultBroadPhaseLayerFilter(layer_),
                           physics_system->GetDefaultLayerFilter(layer_),
                           JPH::BodyFilter(),
                           *physics::Engine::tempAllocator());
}

//---------------------------------------------------------------------------
//! 移動
//---------------------------------------------------------------------------
float3
CharacterImpl::move(f32 delta_time, const float3& move_vector, const float3& old_position, const float3& jump_vector)
{
    float3 move_direction = move_vector;

    //----------------------------------------------------------
    // 階段歩行アルゴリズム
    //----------------------------------------------------------
    if constexpr(true) {
        //------------------------------------------------------
        // 水平方向にどれだけ移動させたいかを計算する
        //------------------------------------------------------

        // 現在の水平方向(XZ成分)の速度ベクトル
        float3 current_horizontal_velocity = linearVelocity() * float3(1.0f, 0.0f, 1.0f) * delta_time;

        // 水平方向のステップの長さ
        f32 velocity_length = dot(current_horizontal_velocity, current_horizontal_velocity);
        if(velocity_length != 0.0f) {
            velocity_length = sqrtf(velocity_length);
        }

        // 実際に水平方向にどれだけ移動したかを計算する
        float3 horizontal_step   = (position() - old_position) * float3(1.0f, 0.0f, 1.0f);
        f32    horizontal_length = dot(horizontal_step, horizontal_step);
        if(horizontal_length != 0.0f) {
            horizontal_length = sqrtf(horizontal_length);
        }

        // もし思うように進まず、急な斜面に面してしまった場合
        if(horizontal_length + 0.0001f < velocity_length && canWalkStairs()) {
            // あとどれだけ前に出ればいいかを計算する
            float3 step_dir     = normalize(current_horizontal_velocity);
            float3 step_forward = step_dir * (velocity_length - horizontal_length);

            // 床をどこまで検査すればいいかを計算する
            static constexpr f32 min_step_forward  = 0.15f;
            float3               step_forward_test = step_dir * min_step_forward;

            static const float3 step_up_height = float3(0.0f, 0.4f, 0.0f);   // 段差の高さ

            // 階段を歩く
            walkStairs(delta_time, step_up_height, step_forward, step_forward_test);
        }
    }

    //------------------------------------------------------
    // 斜面で滑っている時に法線と反対方向の動きをキャンセルする
    //------------------------------------------------------
    auto ground_state = groundState();
    if(ground_state == physics::Character::GroundState::Sliding) {
        float3 normal = normalize(groundNormal() * float3(1.0f, 0.0f, 1.0f));

        f32 dot_result = dot(normal, move_direction);
        if(dot_result < 0.0f)
            move_direction -= normal * dot_result;
    }

    //------------------------------------------------------
    // Y成分の計算
    //------------------------------------------------------
    float3 new_velocity;

    // 現在の垂直方向(Y成分)の速度ベクトル
    float3 current_vertical_velocity = linearVelocity() * float3(0.0f, 1.0f, 0.0f);

    // 移動床の速度
    float3 ground_velocity = groundVelocity();

    f32 relative_vertical_velocity = current_vertical_velocity.y - ground_velocity.y;   // 相対速度

    if(ground_state == physics::Character::GroundState::OnGround   // 地面に接地している時
       && (relative_vertical_velocity < 0.1f))   // かつ、地面から離れようとしていない時
    {
        // 移動床の速度を新しい速度にする
        new_velocity = ground_velocity;

        // ジャンプ
        new_velocity += jump_vector;   // ジャンプ速度
    }
    else {
        new_velocity = current_vertical_velocity;
    }

    return new_velocity;
}

//---------------------------------------------------------------------------
//! 階段を歩けるかどうかを取得
//---------------------------------------------------------------------------
bool CharacterImpl::canWalkStairs() const
{
    return jph_character_->CanWalkStairs();
}

//---------------------------------------------------------------------------
//! 階段歩行
//---------------------------------------------------------------------------
bool CharacterImpl::walkStairs(f32           delta_time,
                               const float3& step_up,
                               const float3& step_forward,
                               const float3& step_forward_test,
                               const float3& step_down_extra)
{
    auto* physics_system = physics::Engine::physicsSystem();

    auto broad_phase_layer_filter = physics_system->GetDefaultBroadPhaseLayerFilter(layer_);
    auto layer_filter             = physics_system->GetDefaultLayerFilter(layer_);

    return jph_character_->WalkStairs(delta_time,
                                      physics_system->GetGravity(),
                                      castJPH(step_up),
                                      castJPH(step_forward),
                                      castJPH(step_forward_test),
                                      castJPH(step_down_extra),
                                      broad_phase_layer_filter,
                                      layer_filter,
                                      {},
                                      *physics::Engine::tempAllocator());
}

//---------------------------------------------------------------------------
//! 衝突点を再検出
//---------------------------------------------------------------------------
void CharacterImpl::refleshContacts()
{
    auto* physics_system = physics::Engine::physicsSystem();

    jph_character_->RefreshContacts(physics_system->GetDefaultBroadPhaseLayerFilter(layer_),
                                    physics_system->GetDefaultLayerFilter(layer_),
                                    JPH::BodyFilter(),
                                    *physics::Engine::tempAllocator());
}

//---------------------------------------------------------------------------
//! キャラクターの形状を入れ替える（しゃがみ、構え方など）
//---------------------------------------------------------------------------
bool CharacterImpl::setShape(const shape::Base& shape, const float3& shape_offset)
{
    assert(shape.shapeType() == shape::Type::Capsule && "カプセル以外は未対応です.");

    //------------------------------------------------------
    // カプセルを生成
    //------------------------------------------------------
    std::unique_ptr<shape::Capsule> s = std::make_unique<shape::Capsule>(static_cast<const shape::Capsule&>(shape));
    JPH::ShapeRefC                  jph_shape = new JPH::CapsuleShape(s->half_height_, s->radius_);

    // 移動されたシェイプを作成
    JPH::ShapeRefC translated_shape =
        JPH::RotatedTranslatedShapeSettings(castJPH(shape_offset), JPH::Quat::sIdentity(), jph_shape).Create().Get();

    //------------------------------------------------------
    // シェイプを切り替え
    // 形状を切り替える前に新しい形状が衝突するかどうかを確認します
    //------------------------------------------------------
    auto* physics_system = physics::Engine::physicsSystem();

    auto broad_phase_layer_filter = physics_system->GetDefaultBroadPhaseLayerFilter(layer_);
    auto layer_filter             = physics_system->GetDefaultLayerFilter(layer_);

    bool is_succeed = jph_character_->SetShape(translated_shape,   // シェイプ
                                               0.1f,   // 切り替え後に許容される最大めり込み量
                                               broad_phase_layer_filter,
                                               layer_filter,
                                               {},
                                               *physics::Engine::tempAllocator());

    if(is_succeed) {
        // 記録内容を更新
        jph_shape_    = jph_shape;
        shape_        = std::move(s);   // 形状
        shape_offset_ = shape_offset;   // シェイプの移動オフセット
    }
    return is_succeed;
}

//---------------------------------------------------------------------------
//! シェイプを取得
//---------------------------------------------------------------------------
shape::Base* CharacterImpl::shape() const
{
    return shape_.get();
}

//---------------------------------------------------------------------------
//! シェイプの移動オフセットを取得
//---------------------------------------------------------------------------
float3 CharacterImpl::shapeOffset() const
{
    return shape_offset_;
}

//---------------------------------------------------------------------------
//! 現在の接地状態を取得
//---------------------------------------------------------------------------
Character::GroundState CharacterImpl::groundState() const
{
    auto state = jph_character_->GetGroundState();

    static constexpr std::array<GroundState, 3> ground_state{
        GroundState::OnGround,
        GroundState::Sliding,
        GroundState::InAir,
    };
    return ground_state[static_cast<s32>(state)];
}

//---------------------------------------------------------------------------
//! 接地している足元の位置座標を取得
//---------------------------------------------------------------------------
float3 CharacterImpl::groundPosition() const
{
    return castJPH(jph_character_->GetGroundPosition());
}

//---------------------------------------------------------------------------
//! 接地している足元の法線を取得
//---------------------------------------------------------------------------
float3 CharacterImpl::groundNormal() const
{
    return castJPH(jph_character_->GetGroundNormal());
}

//---------------------------------------------------------------------------
//! 接地している足元の速度を取得
//---------------------------------------------------------------------------
float3 CharacterImpl::groundVelocity() const
{
    return castJPH(jph_character_->GetGroundVelocity());
}

//---------------------------------------------------------------------------
//! キャラクターが立っているオブジェクトのボディIDを取得
//---------------------------------------------------------------------------
u64 CharacterImpl::groundBodyID() const
{
    return jph_character_->GetGroundBodyID().GetIndexAndSequenceNumber();
}

//---------------------------------------------------------------------------
//! キャラクターが指定されたボディと衝突可能かどうかをチェックします。
//---------------------------------------------------------------------------
bool CharacterImpl::OnContactValidate([[maybe_unused]] const JPH::CharacterVirtual* character,
                                      const JPH::BodyID&                            body_id2,
                                      [[maybe_unused]] const JPH::SubShapeID&       sub_shape_id2)
{
    // デフォルトは常に衝突許可
    if(listener_ == nullptr)
        return true;

    u64 body_id = body_id2.GetIndexAndSequenceNumber();

    // 登録されているリスナーに通知
    return listener_->onContactValidate(this, body_id);
}

//---------------------------------------------------------------------------
//! キャラクターがボディと衝突するたびに呼び出されます
//---------------------------------------------------------------------------
void CharacterImpl::OnContactAdded([[maybe_unused]] const JPH::CharacterVirtual* character,
                                   const JPH::BodyID&                            body_id2,
                                   [[maybe_unused]] const JPH::SubShapeID&       sub_shape_id2,
                                   JPH::Vec3Arg                                  contact_position,
                                   JPH::Vec3Arg                                  contact_normal,
                                   JPH::CharacterContactSettings&                settings)
{
    // デフォルトではなにもしない
    if(listener_ == nullptr)
        return;

    u64 body_id = body_id2.GetIndexAndSequenceNumber();

    // 登録されているリスナーに通知
    physics::Character::ContactSettings result;
    result.can_push_character_   = settings.mCanPushCharacter;
    result.can_receive_impulses_ = settings.mCanReceiveImpulses;

    listener_->onContactAdded(this,
                              body_id,
                              float3(contact_position.GetX(), contact_position.GetY(), contact_position.GetZ()),
                              float3(contact_normal.GetX(), contact_normal.GetY(), contact_normal.GetZ()),
                              result);

    settings.mCanPushCharacter   = result.can_push_character_;
    settings.mCanReceiveImpulses = result.can_receive_impulses_;
}

//===========================================================================
//! @name   生成
//===========================================================================
//@{

// キャラクターを作成
//! @param  [in]    layer   オブジェクトレイヤー(カテゴリー)
std::shared_ptr<physics::Character> createCharacter(u16 layer)
{
    return std::make_shared<physics::CharacterImpl>(layer);
}

//@}

}   // namespace physics
