//---------------------------------------------------------------------------
//! @file   RigidBody.cpp
//! @brief  剛体
//---------------------------------------------------------------------------
#include "PhysicsEngine.h"
#include "PhysicsLayer.h"
#include "RigidBody.h"
#include "System/Physics/Shape.h"

#include <Jolt/Jolt.h>

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
#include <Jolt/Physics/Collision/Shape/ScaledShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Collision/PhysicsMaterialSimple.h>

//---------------------------------------------------------------------------
//! physics::MotionType → JPH::EMotionType へキャスト
//---------------------------------------------------------------------------
JPH::EMotionType convertTo(physics::MotionType type)
{
    static constexpr std::array<JPH::EMotionType, 3> jph_motion_type{
        JPH::EMotionType::Static,
        JPH::EMotionType::Kinematic,
        JPH::EMotionType::Dynamic,
    };
    return jph_motion_type[static_cast<u32>(type)];
}

namespace physics
{

//===========================================================================
//! 剛体クラス
//===========================================================================
class RigidBodyImpl : public physics::RigidBody
{
public:
    RigidBodyImpl(const JPH::ShapeRefC& shape, u16 layer, physics::MotionType motion_type)
    {
        // ボディ自体の設定を作成します。ここで跳ね返り/摩擦係数のような他のプロパティも設定できます。
        auto position = JPH::Vec3(0.0f, 0.0f, 0.0f);
        auto rotation = JPH::Quat::sIdentity();

        JPH::BodyCreationSettings body_settings(shape,                    // 形状
                                                position,                 // 位置座標
                                                rotation,                 // 回転姿勢
                                                convertTo(motion_type),   // 動的/静的/キネマティック
                                                layer);                   // レイヤー番号

        // CCD (Continuous Collision Detection) 有効
        body_settings.mMotionQuality = JPH::EMotionQuality::LinearCast;

        //------------------------------------------------------
        // 剛体を作成
        //------------------------------------------------------
        // ボディを使い果たした場合はnullptrが返ります
        jph_body_ = physics::Engine::bodyInterface()->CreateBody(body_settings);

        if(jph_body_ == nullptr) {
            assert(0 && "ボディ個数を最大数まで使い果たしました。");
            return;
        }

        // ワールドに追加 (アクティブ化)
        physics::Engine::bodyInterface()->AddBody(jph_body_->GetID(), JPH::EActivation::Activate);

        // IDを保存
        body_id_ = jph_body_->GetID();
    }

    //! デストラクタ
    virtual ~RigidBodyImpl()
    {
        // Physicsシステムからボディを削除します。ただしボディ自体はすべての状態を保持しておりいつでも再追加可能です。
        physics::Engine::bodyInterface()->RemoveBody(body_id_);

        // ボディを解放します。これ以降IDが無効になります。
        physics::Engine::bodyInterface()->DestroyBody(body_id_);
    }

    //-----------------------------------------------------------------------
    //! @name   オブジェクトレイヤー（フィルター）
    //-----------------------------------------------------------------------
    //@{

    //! オブジェクトレイヤーを設定
    //! @param  [in]    layer   オブジェクトレイヤー
    virtual void setLayer(u16 layer) override { physics::Engine::bodyInterface()->SetObjectLayer(body_id_, layer); }

    //! オブジェクトレイヤーを取得
    virtual u16 layer() const override { return physics::Engine::bodyInterface()->GetObjectLayer(body_id_); }

    //@}
    //-----------------------------------------------------------------------
    //! @name   位置と回転姿勢
    //-----------------------------------------------------------------------
    //@{

    //! 位置を設定
    //! @param  [in]    position    設定座標
    //! @param  [in]    is_activate アクティブ化するかどうか true:アクティブにする false:アクティブにしない
    virtual void setPosition(const float3& position, bool is_activate = true) override
    {
        physics::Engine::bodyInterface()->SetPosition(body_id_,
                                                      castJPH(position),
                                                      is_activate ? JPH::EActivation::Activate :
                                                                    JPH::EActivation::DontActivate);
    }

    //! 位置を取得
    virtual float3 position() const override
    {
        JPH::Vec3 p = physics::Engine::bodyInterface()->GetPosition(body_id_);
        return castJPH(p);
    }

    //!  重心位置を取得
    virtual float3 centerOfMassPosition() const override
    {
        JPH::Vec3 p = physics::Engine::bodyInterface()->GetCenterOfMassPosition(body_id_);
        return castJPH(p);
    }

    //! 回転姿勢を設定
    //! @param  [in]    rot         設定角度
    //! @param  [in]    is_activate アクティブ化するかどうか true:アクティブにする false:アクティブにしない
    virtual void setRotation(const quaternion& rot, bool is_activate = true) override
    {
        JPH::QuatArg q = castJPH(rot);
        physics::Engine::bodyInterface()->SetRotation(body_id_,
                                                      q,
                                                      is_activate ? JPH::EActivation::Activate :
                                                                    JPH::EActivation::DontActivate);
    }

    //! 回転姿勢を設定
    virtual quaternion rotation() const override
    {
        JPH::QuatArg q = physics::Engine::bodyInterface()->GetRotation(body_id_);
        return castJPH(q);
    }

    //! ワールド行列を取得
    virtual matrix worldMatrix() const override
    {
        return castJPH(physics::Engine::bodyInterface()->GetWorldTransform(body_id_));
    }

    //! 重心の変換行列を取得
    virtual matrix centerOfMassTransform() const override
    {
        return castJPH(physics::Engine::bodyInterface()->GetCenterOfMassTransform(body_id_));
    }

    //@}
    //-----------------------------------------------------------
    //! @name   速度と角速度
    //! @details LinearVelocityは重心の速度であり、物体の位置と一致しない場合がありますので補正してください。
    //-----------------------------------------------------------
    //@{

    //! 速度と角速度を逆算して設定
    //! target_position/target_rotation に delta_time 秒で位置するようにボディの速度を設定する。
    //! 必要に応じてボディをアクティブにします。
    virtual void
    moveKinematic(const float3& target_position, const quaternion& target_rotation, f32 delta_time) override
    {
        JPH::Vec3 tpos = castJPH(target_position);
        JPH::Quat trot = castJPH(target_rotation);
        physics::Engine::bodyInterface()->MoveKinematic(body_id_, tpos, trot, delta_time);
    }

    //! 速度を設定
    //! @param  [in]    v   速度
    virtual void setLinearVelocity(const float3& v) override
    {
        physics::Engine::bodyInterface()->SetLinearVelocity(body_id_, castJPH(v));
    }

    //! 速度を取得
    virtual float3 linearVelocity() const override
    {
        return castJPH(physics::Engine::bodyInterface()->GetLinearVelocity(body_id_));
    }

    //! 現在の速度に速度を加算
    //! @param  [in]    v   加算する速度
    virtual void addLinearVelocity(const float3& v) override
    {
        physics::Engine::bodyInterface()->AddLinearVelocity(body_id_, castJPH(v));
    }

    //! 角速度を設定
    //! @param  [in]    vrot   角速度
    virtual void setAngularVelocity(const float3& vrot) override
    {
        physics::Engine::bodyInterface()->SetAngularVelocity(body_id_, castJPH(vrot));
    }

    //! 角速度を取得
    virtual float3 angularVelocity() const override
    {
        return castJPH(physics::Engine::bodyInterface()->GetAngularVelocity(body_id_));
    }

    //! 指定座標のワールド空間での速度を取得
    //! @param  [in]    point   座標
    virtual float3 pointVelocity(const float3& point) const override
    {
        return castJPH(physics::Engine::bodyInterface()->GetPointVelocity(body_id_, castJPH(point)));
    }

    //@}
    //----------------------------------------------------------
    //! @name 力を与える
    //----------------------------------------------------------
    //@{

    //! 力を与える
    //! @param  [in]    force   与える力
    virtual void addForce(const float3& force) override
    {
        physics::Engine::bodyInterface()->AddForce(body_id_, castJPH(force));
    }

    //! 指定位置に力を与える
    //! @param  [in]    force   与える力
    //! @param  [in]    point   位置
    virtual void addForce(const float3& force, const float3& point) override
    {
        physics::Engine::bodyInterface()->AddForce(body_id_, castJPH(force), castJPH(point));
    }

    //! トルクを与える
    //! @param  [in]    force   与える力
    virtual void addTorque(const float3& torque) override
    {
        physics::Engine::bodyInterface()->AddTorque(body_id_, castJPH(torque));
    }

    //@}
    //----------------------------------------------------------
    //! @name   撃力を与える
    //----------------------------------------------------------
    //@{

    //! 撃力を与える
    //! @param  [in]    impulse 与える撃力
    virtual void addImpulse(const float3& impulse) override
    {
        physics::Engine::bodyInterface()->AddImpulse(body_id_, castJPH(impulse));
    }

    //! 指定位置に撃力を与える
    //! @param  [in]    impulse 与える撃力
    //! @param  [in]    point   位置
    virtual void addImpulse(const float3& impulse, const float3& point) override
    {
        physics::Engine::bodyInterface()->AddImpulse(body_id_, castJPH(impulse), castJPH(point));
    }

    //! 角力積を与える
    //! @param  [in]    impulse 与える角力積
    virtual void addAngularImpulse(const float3& impulse) override
    {
        physics::Engine::bodyInterface()->AddAngularImpulse(body_id_, castJPH(impulse));
    }

    //@}
    //----------------------------------------------------------
    //! @name   跳ね返り係数
    //----------------------------------------------------------
    //@{

    //! 跳ね返り係数を設定
    //! @param  [in]    coeff   跳ね返り係数(0.0f-1.0f)
    virtual void setRestitution(f32 coeff) override
    {
        physics::Engine::bodyInterface()->SetRestitution(body_id_, coeff);
    }

    //! 跳ね返り係数を取得
    virtual f32 restitution() const override { return physics::Engine::bodyInterface()->GetRestitution(body_id_); }

    //@}
    //----------------------------------------------------------
    //! @name   摩擦係数
    //----------------------------------------------------------
    //@{

    //! 摩擦係数を設定
    //! @param  [in]    coeff   摩擦係数(0.0f~)
    virtual void setFriction(f32 coeff) override { physics::Engine::bodyInterface()->SetFriction(body_id_, coeff); }

    //! 摩擦係数を取得
    virtual f32 friction() const override { return physics::Engine::bodyInterface()->GetFriction(body_id_); }

    //@}
    //----------------------------------------------------------
    //! @name   重力係数
    //----------------------------------------------------------
    //@{

    //! 重力係数を設定
    //! @param  [in]    coeff   摩擦係数(0.0f~)
    virtual void setGravityFactor(f32 coeff) override
    {
        physics::Engine::bodyInterface()->SetGravityFactor(body_id_, coeff);
    }

    //! 重力係数を取得
    virtual f32 gravityFactor() const override { return physics::Engine::bodyInterface()->GetGravityFactor(body_id_); }

    //@}

    //! 動的/静的/キネマティックを切り替え
    //! @param  [in]    motion_type 動作タイプ
    //! @param  [in]    is_activate アクティブ化するかどうか true:アクティブにする false:アクティブにしない
    virtual void setMotionType(physics::MotionType motion_type, bool is_activate = true) override
    {
        physics::Engine::bodyInterface()->SetMotionType(body_id_,
                                                        convertTo(motion_type),
                                                        is_activate ? JPH::EActivation::Activate :
                                                                      JPH::EActivation::DontActivate);
    }

    //! ワールド空間の逆慣性テンソルを取得する
    virtual matrix inverseInertia() const override
    {
        return castJPH(physics::Engine::bodyInterface()->GetInverseInertia(body_id_));
    }

    //! ユーザーデーターを設定
    //! @param  [s] ユーザーデーター(識別用に任意に設定可能です)
    virtual void setData(std::intptr_t s) override { data_ = s; }

    //! ユーザーデーターを取得
    virtual std::intptr_t data() const override { return data_; }

    //! ボディIDを取得
    virtual u64 bodyID() const override { return body_id_.GetIndexAndSequenceNumber(); }

private:
    JPH::BodyID   body_id_;              //!< [JPH] ボディID
    JPH::Body*    jph_body_ = nullptr;   //!< [JPH] ボディ
    std::intptr_t data_     = 0;         //!< 任意のデーター
};

//===========================================================================
//! @name   生成
//===========================================================================
//@{

//---------------------------------------------------------------------------
//! Sphere剛体を作成
//---------------------------------------------------------------------------
std::shared_ptr<physics::RigidBody> createRigidBody(const shape::Sphere& o, u16 layer, physics::MotionType motion_type)
{
    // シェイプを生成
    JPH::SphereShapeSettings        shape_settings(o.radius_);
    JPH::ShapeSettings::ShapeResult result = shape_settings.Create();

    if(result.HasError())
        return nullptr;

    // 作成
    return std::make_shared<RigidBodyImpl>(result.Get(), layer, motion_type);
}

//---------------------------------------------------------------------------
//! Box剛体を作成
//---------------------------------------------------------------------------
std::shared_ptr<physics::RigidBody> createRigidBody(const shape::Box& o, u16 layer, physics::MotionType motion_type)
{
    // シェイプを生成
    JPH::BoxShapeSettings           shape_settings(JPH::Vec3(o.extent_.x, o.extent_.y, o.extent_.z));
    JPH::ShapeSettings::ShapeResult result = shape_settings.Create();

    if(result.HasError())
        return nullptr;

    // 作成
    return std::make_shared<RigidBodyImpl>(result.Get(), layer, motion_type);
}

//---------------------------------------------------------------------------
//! Cylinder剛体を作成
//---------------------------------------------------------------------------
std::shared_ptr<physics::RigidBody>
createRigidBody(const shape::Cylinder& o, u16 layer, physics::MotionType motion_type)
{
    // シェイプを生成
    JPH::CylinderShapeSettings      shape_settings(o.half_height_, o.radius_);
    JPH::ShapeSettings::ShapeResult result = shape_settings.Create();

    if(result.HasError())
        return nullptr;

    // 作成
    return std::make_shared<RigidBodyImpl>(result.Get(), layer, motion_type);
}

//---------------------------------------------------------------------------
//! ConvexHull剛体を作成
//---------------------------------------------------------------------------
std::shared_ptr<physics::RigidBody>
createRigidBody(const shape::ConvexHull& o, u16 layer, physics::MotionType motion_type)
{
    //----------------------------------------------------------
    // 頂点配列を取得
    //----------------------------------------------------------
    std::vector<JPH::Vec3>   vertexList;
    JPH::IndexedTriangleList indexList;

    for(auto& v : o.vertices_) {
        JPH::Vec3 position{v.x * 0.01f, v.y * 0.01f, v.z * 0.01f};
        vertexList.emplace_back(std::move(position));
    }

    //----------------------------------------------------------
    // シェイプを生成
    //----------------------------------------------------------
    JPH::ConvexHullShapeSettings    shape_settings(vertexList);
    JPH::ShapeSettings::ShapeResult result = shape_settings.Create();

    if(result.HasError())
        return nullptr;

    // 作成
    return std::make_shared<RigidBodyImpl>(result.Get(), layer, motion_type);
}

//---------------------------------------------------------------------------
//! Mesh剛体を作成
//---------------------------------------------------------------------------
std::shared_ptr<physics::RigidBody> createRigidBody(const shape::Mesh& o, u16 layer)
{
    //----------------------------------------------------------
    // 頂点配列を取得
    //----------------------------------------------------------
    JPH::VertexList          vertexList;
    JPH::IndexedTriangleList indexList;

    for(auto& v : o.vertices_) {
        JPH::Float3 position{v.x, v.y, v.z};
        vertexList.emplace_back(std::move(position));
    }
    for(u32 i = 0; i < o.indices_.size(); i += 3) {
        u32                  i0 = o.indices_[i + 0];
        u32                  i1 = o.indices_[i + 1];
        u32                  i2 = o.indices_[i + 2];
        JPH::IndexedTriangle triangle(i0, i1, i2, 0);
        indexList.emplace_back(std::move(triangle));
    }

    JPH::PhysicsMaterialList materialList;
    materialList.push_back(new JPH::PhysicsMaterialSimple("BPMeshMaterial", JPH::Color::sGetDistinctColor(0)));

    //----------------------------------------------------------
    // シェイプを生成
    //----------------------------------------------------------
    JPH::MeshShapeSettings          shape_settings(vertexList, indexList, materialList);
    JPH::ShapeSettings::ShapeResult result = shape_settings.Create();

    if(result.HasError())
        return nullptr;

    // 作成
    // メッシュ剛体は常に静的で生成されます
    return std::make_shared<RigidBodyImpl>(result.Get(), layer, physics::MotionType::Static);
}

//@}

}   // namespace physics
