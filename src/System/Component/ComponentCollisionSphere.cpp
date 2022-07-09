//---------------------------------------------------------------------------
//! @file   ComponentCollisionSphere.h
//! @brief  Sphereコリジョンコンポーネント
//---------------------------------------------------------------------------
#include <System/Component/ComponentCollisionSphere.h>
#include <System/Component/ComponentCollisionCapsule.h>
#include <System/Component/ComponentCollisionModel.h>
#include <System/Component/ComponentTransform.h>
#include <System/Component/ComponentModel.h>
#include <System/Object.h>
#include <System/Scene.h>

void ComponentCollisionSphere::Init()
{
    __super::Init();
}

void ComponentCollisionSphere::Update(float delta)
{
    __super::Update(delta);
}

// ここで当たり処理後の処理を行います
void ComponentCollisionSphere::PostUpdate()
{
    __super::PostUpdate();
    old_transform_ = GetWorldMatrix();
}

void ComponentCollisionSphere::Draw()
{
    if(!Scene::IsEdit())
        return;

    __super::Draw();

    float scale = 1.0f;
    auto  trans = GetWorldMatrix();

    if(attach_node_ < 0) {
        // モデルアタッチしてない場合のみサイズ変更
        float sx = length(trans.axisVectorX());
        float sy = length(trans.axisVectorY());
        float sz = length(trans.axisVectorZ());
        scale    = (sx + sy + sz) / 3.0f;
    }

    SetUseLighting(FALSE);
    SetLightEnable(FALSE);
    DrawSphere3D(cast(trans.translate()), radius_ * scale, 10, GetColor(0, 255, 0), GetColor(0, 0, 0), FALSE);
    SetLightEnable(TRUE);
    SetUseLighting(TRUE);
}

void ComponentCollisionSphere::Exit()
{
    __super::Exit();
}

//! @brief GUI処理
void ComponentCollisionSphere::GUI()
{
    // 基底クラスのGUIはそのままは使用しない
    //__super::GUI();

    // オーナーの取得
    assert(GetOwner());
    auto obj_name = GetOwner()->GetName();

    ImGui::Begin(obj_name.data());
    {
        ImGui::Separator();
        auto ui_name = std::string("Collision Sphere:") + std::to_string(collision_id_);
        if(ImGui::TreeNode(ui_name.c_str())) {
            // コリジョン情報を出す
            GUICollsionData();

            std::string colname = u8"COL:" + std::to_string(collision_id_) + "/ ";

            float* mat = GetColMatrixFloat();
            float  matrixTranslation[3], matrixRotation[3], matrixScale[3];
            DecomposeMatrixToComponents(mat, matrixTranslation, matrixRotation, matrixScale);
            ImGui::DragFloat3((colname + u8"座標(T)").c_str(), matrixTranslation, 0.01f);
            //ImGui::DragFloat3( u8"COL回転(R)", matrixRotation, 0.1f );
            //ImGui::DragFloat3( u8"COLサイズ(S)", matrixScale, 0.01f );
            ImGuizmo::RecomposeMatrixFromComponents(matrixTranslation, matrixRotation, matrixScale, mat);

            ImGui::DragFloat((colname + u8"半径(R)").c_str(), &radius_, 0.01f, 0.01f, 1000.0f, "%.2f");

            ImGui::TreePop();
        }
    }
    ImGui::End();
}

//! @brief 半径の設定
//! @param radius 半径
ComponentCollisionSpherePtr ComponentCollisionSphere::SetRadius(float radius)
{
    radius_ = radius;
    return std::dynamic_pointer_cast<ComponentCollisionSphere>(shared_from_this());
}

//! @brief 半径の取得
//! @return 半径
float ComponentCollisionSphere::GetRadius() const
{
    return radius_;
}

//! @brief 当たっているかを調べる
//! @param col 相手のコリジョン
//! @return HitInfoを返す
ComponentCollisionSphere::HitInfo ComponentCollisionSphere::IsHit(ComponentCollisionPtr col)
{
    HitInfo info;

    switch(col->GetCollisionType()) {
    case ComponentCollision::CollisionType::LINE:
        // @todo HitCheck_Line_Sphere()
        break;
    case ComponentCollision::CollisionType::TRIANGLE:
        // @todo HitCheck_Sphere_Triangle()
        break;
    case ComponentCollision::CollisionType::SPHERE:
        return isHit(std::dynamic_pointer_cast<ComponentCollisionSphere>(shared_from_this()),
                     std::dynamic_pointer_cast<ComponentCollisionSphere>(col));
        break;
    case ComponentCollision::CollisionType::CAPSULE:
        return isHit(std::dynamic_pointer_cast<ComponentCollisionSphere>(shared_from_this()),
                     std::dynamic_pointer_cast<ComponentCollisionCapsule>(col));
        break;
    case ComponentCollision::CollisionType::MODEL:
        return isHit(std::dynamic_pointer_cast<ComponentCollisionSphere>(shared_from_this()),
                     std::dynamic_pointer_cast<ComponentCollisionModel>(col));
        break;
    }

    return info;
}

//! @brief ワールドMatrixの取得
//! @return 他のコンポーネントも含めた位置
const matrix ComponentCollisionSphere::GetWorldMatrix()
{
    auto transform = collision_transform_;

    auto obj = GetOwner();
    if(attach_node_ >= 0) {
        auto mdl = obj->GetComponent<ComponentModel>();
        if(mdl) {
            transform = mul(transform, attach_node_matrix_);
        }
    }
    else {
        auto cmp = obj->GetComponent<ComponentTransform>();
        if(cmp) {
            transform = mul(transform, cmp->GetMatrix());
        }
    }

    return transform;
}
