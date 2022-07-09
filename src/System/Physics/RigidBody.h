//---------------------------------------------------------------------------
//! @file   RigidBody.h
//! @brief  剛体
//---------------------------------------------------------------------------
#pragma once

namespace shape
{
class Sphere;           //!< 球
class Box;              //!< ボックス
class HeightField;      //!< 高さマップ
class Mesh;             //!< 静的メッシュ
class ConvexHull;       //!< 凸形状
class Capsule;          //!< カプセル
class TaperedCapsule;   //!< 両端半径が異なるカプセル
class Cylinder;         //!< 円筒
class Triangle;         //!< 三角形
class StaticCompound;
class MutableCompound;
}   // namespace shape

namespace physics
{

//--------------------------------------------------------------
//! 動作タイプ
//--------------------------------------------------------------
enum class MotionType
{
    Static,      //!< 静的。動かない。
    Kinematic,   //!< 速度のみで移動可能で、力には反応しない。
    Dynamic,     //!< 通常の物理オブジェクトとして力に応答する。
};

//===========================================================================
//! 剛体クラス
//===========================================================================
class RigidBody
{
public:
    //! デストラクタ
    virtual ~RigidBody() = default;

    //-----------------------------------------------------------------------
    //! @name   オブジェクトレイヤー（フィルター）
    //-----------------------------------------------------------------------
    //@{

    //! オブジェクトレイヤーを設定
    //! @param  [in]    layer   オブジェクトレイヤー
    virtual void setLayer(u16 layer) = 0;

    //! オブジェクトレイヤーを取得
    virtual u16 layer() const = 0;

    //@}
    //-----------------------------------------------------------------------
    //! @name   位置と回転姿勢
    //-----------------------------------------------------------------------
    //@{

    //! 位置を設定
    //! @param  [in]    position    設定座標
    //! @param  [in]    is_activate アクティブ化するかどうか true:アクティブにする false:アクティブにしない
    virtual void setPosition(const float3& position, bool is_activate = true) = 0;

    //! 位置を取得
    virtual float3 position() const = 0;

    //! 重心位置を取得
    virtual float3 centerOfMassPosition() const = 0;

    //! 回転姿勢を設定
    //! @param  [in]    rot         設定角度
    //! @param  [in]    is_activate アクティブ化するかどうか true:アクティブにする false:アクティブにしない
    virtual void setRotation(const quaternion& rot, bool is_activate = true) = 0;

    //! 回転姿勢を設定
    virtual quaternion rotation() const = 0;

    //! ワールド行列を取得
    virtual matrix worldMatrix() const = 0;

    //! 重心の変換行列を取得
    virtual matrix centerOfMassTransform() const = 0;

    //@}
    //-----------------------------------------------------------
    //! @name   速度と角速度
    //! @details LinearVelocityは重心の速度であり、物体の位置と一致しない場合がありますので補正してください。
    //-----------------------------------------------------------
    //@{

    //! 速度と角速度を逆算して設定
    //! @param  [in]    target_position 目標の位置
    //! @param  [in]    target_rotation 目標の回転姿勢
    //! @param  [in]    delta_time      所要時間
    //! target_position/target_rotation に delta_time 秒で位置するようにボディの速度を設定します。
    //! 必要に応じてボディをアクティブにします。
    virtual void moveKinematic(const float3& target_position, const quaternion& target_rotation, f32 delta_time) = 0;

    //! 速度を設定
    //! @param  [in]    v   速度
    virtual void setLinearVelocity(const float3& v) = 0;

    //! 速度を取得
    virtual float3 linearVelocity() const = 0;

    //! 現在の速度に速度を加算
    //! @param  [in]    v   加算する速度
    virtual void addLinearVelocity(const float3& v) = 0;

    //! 角速度を設定
    //! @param  [in]    vrot   角速度
    virtual void setAngularVelocity(const float3& vrot) = 0;

    //! 角速度を取得
    virtual float3 angularVelocity() const = 0;

    //! 指定座標のワールド空間での速度を取得
    //! @param  [in]    point   座標
    virtual float3 pointVelocity(const float3& point) const = 0;

    //@}
    //----------------------------------------------------------
    //! @name 力を与える
    //----------------------------------------------------------
    //@{

    //! 力を与える
    //! @param  [in]    force   与える力
    virtual void addForce(const float3& force) = 0;

    //! 指定位置に力を与える
    //! @param  [in]    force   与える力
    //! @param  [in]    point   位置
    virtual void addForce(const float3& force, const float3& point) = 0;

    //! トルクを与える
    //! @param  [in]    force   与える力
    virtual void addTorque(const float3& torque) = 0;

    //@}
    //----------------------------------------------------------
    //! @name   撃力を与える
    //----------------------------------------------------------
    //@{

    //! 撃力を与える
    //! @param  [in]    impulse 与える撃力
    virtual void addImpulse(const float3& impulse) = 0;

    //! 指定位置に撃力を与える
    //! @param  [in]    impulse 与える撃力
    //! @param  [in]    point   位置
    virtual void addImpulse(const float3& impulse, const float3& point) = 0;

    //! 角力積を与える
    //! @param  [in]    angular_impulse 与える角力積
    virtual void addAngularImpulse(const float3& impulse) = 0;

    //@}
    //----------------------------------------------------------
    //! @name   跳ね返り係数
    //----------------------------------------------------------
    //@{

    //! 跳ね返り係数を設定
    //! @param  [in]    coeff   跳ね返り係数(0.0f-1.0f)
    virtual void setRestitution(f32 coeff) = 0;

    //! 跳ね返り係数を取得
    virtual f32 restitution() const = 0;

    //@}
    //----------------------------------------------------------
    //! @name   摩擦係数
    //----------------------------------------------------------
    //@{

    //! 摩擦係数を設定
    //! @param  [in]    coeff   摩擦係数(0.0f~)
    virtual void setFriction(f32 coeff) = 0;

    //! 摩擦係数を取得
    virtual f32 friction() const = 0;

    //@}
    //----------------------------------------------------------
    //! @name   重力係数
    //----------------------------------------------------------
    //@{

    //! 重力係数を設定
    //! @param  [in]    coeff   摩擦係数(0.0f~)
    virtual void setGravityFactor(f32 coeff) = 0;

    //! 重力係数を取得
    virtual f32 gravityFactor() const = 0;

    //@}

    //! 動的/静的/キネマティックを切り替え
    //! @param  [in]    motion_type 動作タイプ
    //! @param  [in]    is_activate アクティブ化するかどうか true:アクティブにする false:アクティブにしない
    virtual void setMotionType(physics::MotionType motion_type, bool is_activate = true) = 0;

    //! ワールド空間の逆慣性テンソルを取得する
    virtual matrix inverseInertia() const = 0;

    //! ユーザーデーターを設定
    //! @param  [s] ユーザーデーター(識別用に任意に設定可能です)
    virtual void setData(std::intptr_t s) = 0;

    //! ユーザーデーターを取得
    virtual std::intptr_t data() const = 0;

    //! ボディIDを取得
    virtual u64 bodyID() const = 0;
};

//===========================================================================
//! @name   生成
//===========================================================================
//@{

//  Sphere剛体を作成
std::shared_ptr<physics::RigidBody> createRigidBody(const shape::Sphere& o,       //
                                                    u16                  layer,   //
                                                    physics::MotionType  motion_type = physics::MotionType::Dynamic);

//  Box剛体を作成
std::shared_ptr<physics::RigidBody> createRigidBody(const shape::Box&   o,       //
                                                    u16                 layer,   //
                                                    physics::MotionType motion_type = physics::MotionType::Dynamic);

//  Cylinder剛体を作成
std::shared_ptr<physics::RigidBody> createRigidBody(const shape::Cylinder& o,       //
                                                    u16                    layer,   //
                                                    physics::MotionType    motion_type = physics::MotionType::Dynamic);

//  ConvexHull剛体を作成
std::shared_ptr<physics::RigidBody> createRigidBody(const shape::ConvexHull& o,       //
                                                    u16                      layer,   //
                                                    physics::MotionType motion_type = physics::MotionType::Dynamic);

//  Mesh剛体を作成
//! @note   メッシュ剛体は常に静的で生成されます
std::shared_ptr<physics::RigidBody> createRigidBody(const shape::Mesh& o, u16 layer);

//@}

}   // namespace physics
