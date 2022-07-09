//---------------------------------------------------------------------------
//! @file   ComponentCollision.h
//! @brief  コリジョンコンポーネント(ベースクラス)
//---------------------------------------------------------------------------
#pragma once
#include <System/Component/Component.h>
#include <ImGuizmo/ImGuizmo.h>
#include <DxLib.h>

#ifdef USE_JOLT_PHYSICS
#include <System/Physics/PhysicsEngine.h>
#include <System/Physics/PhysicsLayer.h>
#include <System/Physics/RigidBody.h>
#include <System/Physics/Shape.h>
#include <System/Physics/PhysicsCharacter.h>
using RigidBodyPtr = std::shared_ptr<physics::RigidBody>;
#endif

USING_PTR(ComponentCollision);
USING_PTR(ComponentCollisionCapsule);
USING_PTR(ComponentCollisionSphere);
USING_PTR(ComponentCollisionModel);

//! @brief モデルコンポーネントクラス
class ComponentCollision : public Component
{
public:
    //! @brief ヒット情報
    struct HitInfo
    {
        bool                  hit_           = false;                //!< ヒットしたか
        ComponentCollisionPtr collision_     = nullptr;              //!< 自分のコリジョン
        float3                push_          = {0.0f, 0.0f, 0.0f};   //!< めり込み量
        float3                hit_position_  = {0.0f, 0.0f, 0.0f};   //!< 当たった地点
        ComponentCollisionPtr hit_collision_ = nullptr;              //!< 当たったコリジョン
    };

    ComponentCollision(ObjectPtr owner);

    virtual void Update(float delta) override;   //!< Update
    virtual void PostUpdate() override;          //!< PostUpdate
    virtual void GUI() override;                 //!< GUI

    void GUICollsionData(bool use_attach = true);

    //! @brief 当たりをチェックします
    //! @param collision 相手コリジョン
    //! @return 当たりの情報
    virtual HitInfo IsHit(ComponentCollisionPtr collision)
    {
        // オーバーライドしてください
        return HitInfo();
    }

    //! @brief 当たった情報はコールバックで送られてくる
    //! @param hitInfo 当たった情報
    //! @details 当たった回数分ここに来ます
    virtual void OnHit(const HitInfo& hitInfo);

    //---------------------------------------------------------------------------
    //! コリジョンステータス
    //---------------------------------------------------------------------------
    enum struct CollisionBit : u32
    {
        Initialized,   //!< 初期化済み
        DisableHit,    //!< 当たらない
        ShowInGame,    //!< ゲーム中にも当たりが見える
        IsGround,      //!< グランド上にいる
        UsePhysics,    //!< 移動でPhysicsが有効になります
    };

    bool IsCollisionStatus(CollisionBit bit) { return collision_status_.is(bit); }
    void SetCollisionStatus(CollisionBit bit, bool b) { collision_status_.set(bit, b); }

    //! @brief タイプは増えたら登録する必要がある
    enum struct CollisionType : u32
    {
        NONE = (u32)-1,
        LINE = 0,
        TRIANGLE,
        SPHERE,
        CAPSULE,
        MODEL,
    };

    //! @brief コリジョンタイプ名
    const std::vector<std::string> CollisionTypeName{
        "LINE",       //< ライン
        "TRIANGLE",   //< トライアングル
        "SPHERE",     //< スフィア
        "CAPSULE",    //< カプセル
        "MODEL",      //< モデル
    };

    enum struct CollisionGroup : u32
    {
        WALL   = 1 << 0,
        GROUND = 1 << 1,
        PLAYER = 1 << 2,
        ENEMY  = 1 << 3,
        WEAPON = 1 << 4,
        ITEM   = 1 << 5,
        CAMERA = 1 << 6,
        ETC    = 1 << 7,
#if 0
		WALL2	= 1 << 8,
		GROUND2	= 1 << 9,
		PLAYER2 = 1 << 10,
		ENEMY2	= 1 << 11,
		WEAPON2 = 1 << 12,
		ITEM2	= 1 << 13,
		CAMERA2 = 1 << 14,
		ETC2	= 1 << 15,
#endif
    };

    std::string GetCollisionGroupName() const
    {
#pragma warning(disable : 26813)
        CollisionGroup grp = collision_group_;

        // この関数では「grp」はBITを前提としておらず
        // このタイプの名前を知る必要があり、もし&判定した場合、仮にビットが2つついていると
        // 正しい値として返してしまうためワーニングを抑制します

        if(grp == CollisionGroup::WALL)
            return "WALL";
        if(grp == CollisionGroup::GROUND)
            return "GROUND";
        if(grp == CollisionGroup::PLAYER)
            return "PLAYER";
        if(grp == CollisionGroup::ENEMY)
            return "ENEMY";
        if(grp == CollisionGroup::WEAPON)
            return "WEAPON";
        if(grp == CollisionGroup::ITEM)
            return "ITEM";
        if(grp == CollisionGroup::CAMERA)
            return "CAMERA";
        if(grp == CollisionGroup::ETC)
            return "ETC";

        // 登録し忘れの可能性があります
        return "UNKNOWN";
#pragma warning(default : 26813)
    }

    inline const CollisionType GetCollisionType() const
    {
        return collision_type_;
    }
    inline void SetHitCollisionGroup(u32 hit_group)
    {
        collision_hit_ = hit_group;
    }

    inline const CollisionGroup GetCollisionGroup() const
    {
        return collision_group_;
    }
    inline void SetCollisionGroup(CollisionGroup grp)
    {
        collision_group_ = grp;
    }

    inline void SetMass(float mass)
    {
        collision_mass_ = mass;
    }
    inline float GetMass() const
    {
        return collision_mass_;
    }

    inline u32 GetId() const
    {
        return collision_id_;
    }

    //----------------------------------------------------------
    // @name アタッチ関係
    //----------------------------------------------------------
    void AttachToModel(int node);

#if 1
    //! @brief コリジョンマトリクス
    float* GetColMatrixFloat()
    {
        return collision_transform_.f32_128_0;
    }

    float3 GetColMatrixTranslate() const
    {
        return collision_transform_._41_42_43;
    }
#endif

    //! @brief  押し戻し量の再計算
    //! @param collision        相手のコリジョン
    //! @param vec              自分100%の戻り量
    //! @param push [out]       本当の自分の押し戻し量
    //! @param other_push [out] 相手の押し戻し量
    //! @return 押し戻しできる状態か?
    bool CalcPush(ComponentCollisionPtr collision, float3 vec, float3* push, float3* other_push) const
    {
        float mass1 = GetMass();
        float mass2 = collision->GetMass();
        if(mass1 < 0 && mass2 < 0) {
            // 両方不動のものがぶつかった
            *push       = vec * 0.5f;
            *other_push = -vec * 0.5f;
            return false;
        }
        else if(mass1 < 0) {
            // 相手に跳ね返す
            *push       = {0, 0, 0};
            *other_push = -vec;
        }
        else if(mass2 < 0) {
            // 相手に跳ね返す
            *push       = vec;
            *other_push = {0, 0, 0};
        }
        else {
            // 重さに合わせて移動させる
            mass1 += 0.001f;
            mass2 += 0.001f;
            float all   = mass1 + mass2;
            *push       = vec * mass2 / all;
            *other_push = -vec * mass1 / all;
        }
        return true;
    }

    bool IsGroupHit(ComponentCollisionPtr collision) const
    {
        // コリジョンが当たらない設定になっている
        if(collision_status_.is(CollisionBit::DisableHit) || collision->collision_status_.is(CollisionBit::DisableHit))
            return false;

        // 自分か相手のコリジョンタイプが不定
        if(collision_type_ == CollisionType::NONE || collision->collision_type_ == CollisionType::NONE)
            return false;

        // 自分は相手のコリジョングループに当たらない
        if(((u32)collision_group_ & collision->collision_hit_) == 0)
            return false;

        // 相手は自分のコリジョングループに当たらない
        if(((u32)collision->collision_group_ & collision_hit_) == 0)
            return false;

        // 当たる設定となる
        return true;
    }

    void UseGravity(bool b = true)
    {
        use_gravity_ = b;
    }

    bool IsUseGravity()
    {
        return use_gravity_;
    }

protected:
    //----------------------------------------------------------------------------
    //! @name 当たり判定処理
    //----------------------------------------------------------------------------
    //@{

    //! @brief Capsule VS Sphere
    //! @param col1 Capsuleコリジョン
    //! @param col2 Sphere コリジョン
    //! @return 当たり情報
    ComponentCollision::HitInfo isHit(ComponentCollisionCapsulePtr col1, ComponentCollisionSpherePtr col2);

    //! @brief Sphere VS Capsule
    //! @param col1 Sphereコリジョン
    //! @param col2 Capsule コリジョン
    //! @return 当たり情報
    ComponentCollision::HitInfo isHit(ComponentCollisionSpherePtr col1, ComponentCollisionCapsulePtr col2);

    //! @brief Capsule VS Capsule
    //! @param col1 Capsuleコリジョン
    //! @param col2 Capsule コリジョン
    //! @return 当たり情報
    ComponentCollision::HitInfo isHit(ComponentCollisionCapsulePtr col1, ComponentCollisionCapsulePtr col2);

    //! @brief Sphere VS Sphere
    //! @param col1 Sphereコリジョン
    //! @param col2 Sphere コリジョン
    //! @return 当たり情報
    ComponentCollision::HitInfo isHit(ComponentCollisionSpherePtr col1, ComponentCollisionSpherePtr col2);

    //! @brief Model VS Sphere
    //! @param col1 Modelコリジョン
    //! @param col2 Sphere コリジョン
    //! @return 当たり情報
    ComponentCollision::HitInfo isHit(ComponentCollisionModelPtr col1, ComponentCollisionSpherePtr col2);

    //! @brief Sphere VS Model
    //! @param col1 Shpereコリジョン
    //! @param col2 Model コリジョン
    //! @return 当たり情報
    ComponentCollision::HitInfo isHit(ComponentCollisionSpherePtr col1, ComponentCollisionModelPtr col2);

    //! @brief Model VS Capsule
    //! @param col1 Modelコリジョン
    //! @param col2 Capsule コリジョン
    //! @return 当たり情報
    ComponentCollision::HitInfo isHit(ComponentCollisionModelPtr col1, ComponentCollisionCapsulePtr col2);

    //! @brief Capsule VS Model
    //! @param col1 Capsuleコリジョン
    //! @param col2 Model コリジョン
    //! @return 当たり情報
    ComponentCollision::HitInfo isHit(ComponentCollisionCapsulePtr col1, ComponentCollisionModelPtr col2);

    //@}

    //! コリジョン用のトランスフォーム
    matrix collision_transform_ = matrix::identity();

    //! 1フレーム前の状態 (WorldTransform)
    matrix old_transform_ = matrix::identity();

    CollisionType  collision_type_  = CollisionType::NONE;
    CollisionGroup collision_group_ = CollisionGroup::ETC;   //!< 自分のコリジョンタイプ
    u32            collision_hit_   = 0xffffffff;            //!< デフォルトではすべてに当たる

    Status<CollisionBit> collision_status_;   //!< 状態

    float collision_mass_ = 1;   //!< 押し戻される量に影響(マイナスは戻されない)
    u32   collision_id_   = 0;   //!< コリジョン識別子

    int    attach_node_        = -1;   //!< モデルノードに付くときは0以上
    matrix attach_node_matrix_ = matrix::identity();

    bool   use_gravity_ = false;
    float3 gravity_     = {0.0f, -0.98f, 0.0f};
    float3 now_gravity_ = {0.0f, 0.0f, 0.0f};

#ifdef USE_JOLT_PHYSICS

public:
    RigidBodyPtr& RigidBody()
    {
        return rigid_body_;
    }
    RigidBodyPtr GetRigidBody() const
    {
        return rigid_body_;
    }
    void SetRigidBody(RigidBodyPtr ptr)
    {
        rigid_body_ = ptr;
    }

protected:
    RigidBodyPtr rigid_body_{};
#endif

private:
    //--------------------------------------------------------------------
    //! @name Cereal処理
    //--------------------------------------------------------------------
    //@{
    CEREAL_SAVELOAD(arc, ver)
    {
        arc(cereal::make_nvp("owner", owner_),
            cereal::make_nvp("collision_type", (u32)collision_type_),
            cereal::make_nvp("collision_group", (u32)collision_group_),
            cereal::make_nvp("collision_hit", collision_hit_),
            cereal::make_nvp("collision_mass", collision_mass_),
            cereal::make_nvp("collision_id", collision_id_),
            cereal::make_nvp("attach_node", attach_node_),
            cereal::make_nvp("attach_node_matrix", attach_node_matrix_),
            cereal::make_nvp("collision_transform", collision_transform_),
            cereal::make_nvp("old_transform", old_transform_));
    }

    CEREAL_LOAD_AND_CONSTRUCT(ComponentCollision, arc, ver)
    {
        ObjectPtr owner;

        arc(CEREAL_NVP(owner));

        construct(owner);

        arc(cereal::make_nvp("collision_type", (u32)construct->collision_type_),
            cereal::make_nvp("collision_group", (u32)construct->collision_group_),
            cereal::make_nvp("collision_hit", construct->collision_hit_),
            cereal::make_nvp("collision_status", construct->collision_status_.get()),
            cereal::make_nvp("collision_mass", construct->collision_mass_),
            cereal::make_nvp("collision_id", construct->collision_id_),
            cereal::make_nvp("attach_node_", construct->attach_node_),
            cereal::make_nvp("attach_node_matrix", construct->attach_node_matrix_),
            cereal::make_nvp("collision_transform", construct->collision_transform_),
            cereal::make_nvp("old_transform", construct->old_transform_));
    }
    //@}
};

CEREAL_REGISTER_TYPE(ComponentCollision)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Component, ComponentCollision)
