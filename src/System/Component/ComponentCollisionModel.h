//---------------------------------------------------------------------------
//! @file   ComponentCollisionModel.h
//! @brief  コリジョンコンポーネント
//---------------------------------------------------------------------------
#pragma once
#include <System/Component/ComponentCollision.h>
#include <System/Component/ComponentTransform.h>
#include <ImGuizmo/ImGuizmo.h>
#include <DxLib.h>

USING_PTR(ComponentCollisionModel);

//! @brief コリジョンコンポーネントクラス
class ComponentCollisionModel
    : public ComponentCollision
    , public IMatrix<ComponentCollisionModel>
{
public:
    ComponentCollisionModel(ObjectPtr owner)
        : ComponentCollision(owner)
    {
        collision_type_  = CollisionType::MODEL;
        collision_mass_  = -1;   //< 移動しないように
        collision_group_ = CollisionGroup::GROUND;
    }

    virtual void Init() override;
    virtual void Update(float delta) override;
    virtual void PostUpdate() override;
    virtual void Draw() override;
    virtual void Exit() override;

    virtual void GUI() override;   //!< GUI

    // 現在のモデルにアタッチします
    void AttachToModel();

    HitInfo IsHit(ComponentCollisionPtr col) override;

    //----------------------------------------------------------------------
    //! @name IMatrixインターフェースの利用するための定義
    //----------------------------------------------------------------------
    //@{
    //! @brief TransformのMatrix情報を取得します
    //! @return ComponentTransform の Matrix
    matrix& Matrix() override { return collision_transform_; }

    virtual ComponentCollisionModelPtr SharedThis() override
    {
        return std::dynamic_pointer_cast<ComponentCollisionModel>(shared_from_this());
    }

    //! @brief ワールドMatrixの取得
    //! @return 他のコンポーネントも含めた位置
    virtual const matrix GetWorldMatrix() override;

    //! @brief 1フレーム前のワールドMatrixの取得
    //! @return 他のコンポーネントも含めた位置
    virtual const matrix GetOldWorldMatrix() override { return old_transform_; }
    //@}

    //! @brief マップに対する正確な移動量を割り出す
    //! @param vec 位置
    //! @param vec 動きたい移動量
    //! @return 実際動ける量
    float3 checkMovement(float3 pos, float3 vec, float force = 1.0f);

private:
    MV1_REF_POLYGONLIST                   ref_poly_{};      //!< ポリゴンデータ
    std::vector<MV1_COLL_RESULT_POLY_DIM> hit_poly_dims_;   //!< 結果代入用ポリゴン配列

    //--------------------------------------------------------------------
    //! @name Cereal処理
    //--------------------------------------------------------------------
    //@{
    CEREAL_SAVELOAD(arc, ver) { arc(cereal::make_nvp("owner", owner_)); }

    CEREAL_LOAD_AND_CONSTRUCT(ComponentCollisionModel, arc, ver)
    {
        ObjectPtr owner;

        arc(CEREAL_NVP(owner));

        construct(owner);
    }
    //@}
};

CEREAL_REGISTER_TYPE(ComponentCollisionModel)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Component, ComponentCollisionModel)
