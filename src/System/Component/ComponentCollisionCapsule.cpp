//---------------------------------------------------------------------------
//! @file   ComponentCollisionCapsule.h
//! @brief  Capsuleコリジョンコンポーネント
//---------------------------------------------------------------------------
#include <System/Component/ComponentCollisionSphere.h>
#include <System/Component/ComponentCollisionCapsule.h>
#include <System/Component/ComponentCollisionModel.h>
#include <System/Component/ComponentModel.h>
#include <System/Component/ComponentTransform.h>
#include <System/Object.h>
#include <System/Scene.h>

void ComponentCollisionCapsule::Init()
{
    __super::Init();
#ifdef USE_JOLT_PHYSICS
    float half_height = (height_ - radius_ * 2.0f) * 0.5f;

    character_ = physics::createCharacter(physics::ObjectLayers::MOVING);
    float3 pos = GetTranslate();
    character_->setShape(shape::Capsule(half_height, radius_), pos);
    set_size_ = false;

    listener_.SetComponent(std::dynamic_pointer_cast<ComponentCollisionCapsule>(shared_from_this()));

    character_->setListener(&listener_);

    // 作成したときはPhysicsでの移動はしない
    SetCollisionStatus(CollisionBit::UsePhysics, false);
#endif   //USE_JOLT_PHYSICS
}

void ComponentCollisionCapsule::Update(float delta)
{
#ifdef USE_JOLT_PHYSICS
    if(set_size_)
        Init();
#endif   //USE_JOLT_PHYSICS

    __super::Update(delta);
}

void ComponentCollisionCapsule::PrePhysics()
{
#ifdef USE_JOLT_PHYSICS
    __super::PrePhysics();

    auto owner = GetOwner();
    auto mat   = mul(Matrix(), owner->GetMatrix());

    if(update_delta_time_ <= 0.0f)
        update_delta_time_ = 1.0f / 60.0f;

    float3 vec = {0, 0, 0};
    if(IsCollisionStatus(CollisionBit::UsePhysics)) {
        // Physicsでの移動
        vec = mat._41_42_43 - mul(Matrix(), owner->GetWorldMatrix())._41_42_43;
        vec *= (1.0f / update_delta_time_);
    }
    else {
        // ワープ
        character_->setPosition(mat._41_42_43);
        // Physics移動に変更
        SetCollisionStatus(CollisionBit::UsePhysics, true);
    }
#if 1
    character_->setRotation(   // rotation
        quaternion(            // quaternion {
            {
                {mat._11_12_13},   //
                {mat._21_22_23},   //
                {mat._31_32_33},   //
            })                     // } quaternion
    );
#endif
    auto* physicsEngine = physics::Engine::instance();
    //physicsEngine->setGravity( {0,-0.98,0} );

    // OwnerのOldMatrixと現在のMatrixから移動量を割り出す
    auto old_pos = GetOwner()->GetOldWorldMatrix().translate();
    auto mov     = GetOwner()->GetTranslate() - old_pos;

    float3 new_velocity = mov * (1.0f / update_delta_time_);

#if 1
    // 重力
    if(use_gravity_)
        gravity_ += physicsEngine->gravity() * 0.1f;   //< キャラが重力にて滑るため、0.1倍にしています

    // プレイヤー操作
    new_velocity += vec + gravity_;
    //character_->setMass( 600.0f );

    // 速度を更新
    character_->setLinearVelocity(new_velocity);

    character_->update(update_delta_time_);
#endif
#endif   //USE_JOLT_PHYSICS
}

// ここで当たり処理後の処理を行います
void ComponentCollisionCapsule::PostUpdate()
{
    __super::PostUpdate();

#ifdef USE_JOLT_PHYSICS
    // Matrixに合わせて親を逆計算
    auto mat = (character_->worldMatrix());

    auto pos = character_->position();

    auto own_mat = inverse(Matrix());

    // Collision中心位置の違いの調整
    //float half_height = ( height_ - radius_ * 2 ) * 0.5f;
    //own_mat._41_42_43 -= ( own_mat._21_22_23 * ( half_height + radius_ ) );

    GetOwner()->Matrix() = mat;
    //mul( own_mat, mat );
#endif
}

void ComponentCollisionCapsule::Draw()
{
    if(!Scene::IsEdit())
        return;

#ifdef USE_JOLT_PHYSICS
    if(character_ == nullptr)
        return;

    float half_height = (height_ - radius_ * 2) * 0.5f;

    auto mat = matrix(character_->rotation());
    mat      = mul(mat, matrix::translate(float3{0, half_height + radius_, 0}));
    mat      = mul(mat, Matrix());

    float3 pos = mul(float4(character_->position(), 1), mat).xyz;

    auto top    = pos + mat.axisY() * half_height;
    auto bottom = pos - mat.axisY() * half_height;

    SetUseLighting(FALSE);
    SetLightEnable(FALSE);
    DrawCapsule3D(cast(top), cast(bottom), radius_, 10, GetColor(0, 255, 0), GetColor(0, 0, 0), FALSE);
    SetUseLighting(TRUE);
    SetLightEnable(TRUE);
#else
    __super::Draw();

    float scale = 1.0f;

    auto trans = GetWorldMatrix();

    if(attach_node_ < 0) {
        // モデルアタッチしてない場合のみサイズ変更
        float sx = length(trans.axisVectorX());
        float sy = length(trans.axisVectorY());
        float sz = length(trans.axisVectorZ());
        scale    = (sx + sy + sz) / 3.0f;
    }

    float radius = radius_ * scale;

    SetUseLighting(FALSE);
    SetLightEnable(FALSE);

    if(height_ < radius * 2)
        height_ = radius * 2;

    float3 pos2 = trans.translate();
    float3 pos1 = normalize((float3&)trans.axisVectorY()) * height_ + pos2;

    float3 vec = normalize(pos2 - pos1);

    DrawCapsule3D(cast(pos1 + vec * radius),
                  cast(pos2 - vec * radius),
                  radius,
                  10,
                  GetColor(0, 255, 0),
                  GetColor(0, 0, 0),
                  FALSE);
    SetLightEnable(TRUE);
    SetUseLighting(TRUE);
#endif
}

void ComponentCollisionCapsule::Exit()
{
    __super::Exit();
}

//! @brief GUI処理
void ComponentCollisionCapsule::GUI()
{
    // 基底クラスのGUIはそのままは使用しない
    //__super::GUI();

    // オーナーの取得
    assert(GetOwner());
    auto obj_name = GetOwner()->GetName();

    ImGui::Begin(obj_name.data());
    {
        ImGui::Separator();

        auto ui_name = std::string("Collision Capsule:") + std::to_string(collision_id_);

        if(ImGui::TreeNode(ui_name.c_str())) {
            // コリジョン情報を出す
            GUICollsionData();

            std::string colname = u8"COL:" + std::to_string(collision_id_) + "/ ";

            float* mat = GetMatrixFloat();
            float  matrixTranslation[3], matrixRotation[3], matrixScale[3];
            DecomposeMatrixToComponents(mat, matrixTranslation, matrixRotation, matrixScale);
            ImGui::DragFloat3((colname + u8"座標(T)").c_str(), matrixTranslation, 0.01f);
            ImGui::DragFloat3((colname + u8"回転(R)").c_str(), matrixRotation, 0.1f);
            //ImGui::DragFloat3( u8"COLサイズ(S)", matrixScale, 0.01f );
            ImGuizmo::RecomposeMatrixFromComponents(matrixTranslation, matrixRotation, matrixScale, mat);

            ImGui::DragFloat((colname + u8"半径(R)").c_str(), &radius_, 0.01f, 0.01f, 1000.0f, "%.2f");

            ImGui::DragFloat((colname + u8"高さ(H)").c_str(), &height_, 0.01f, 0.01f, 1000.0f, "%.2f");

            ImGui::TreePop();
        }
    }
    ImGui::End();
}

//! @brief 半径の設定
//! @param radius 半径
ComponentCollisionCapsulePtr ComponentCollisionCapsule::SetRadius(float radius)
{
#ifdef USE_JOLT_PHYSICS
    set_size_ = true;
#endif   //USE_JOLT_PHYSICS
    radius_ = radius;
    return std::dynamic_pointer_cast<ComponentCollisionCapsule>(shared_from_this());
}

//! @brief 半径の取得
//! @return 半径
float ComponentCollisionCapsule::GetRadius() const
{
    return radius_;
}

ComponentCollisionCapsulePtr ComponentCollisionCapsule::SetHeight(float height)
{
#ifdef USE_JOLT_PHYSICS
    set_size_ = true;
#endif   //USE_JOLT_PHYSICS
    height_ = height;
    return std::dynamic_pointer_cast<ComponentCollisionCapsule>(shared_from_this());
}

float ComponentCollisionCapsule::GetHeight()
{
    return height_;
}

//! @brief 当たっているかを調べる
//! @param col 相手のコリジョン
//! @return HitInfoを返す
ComponentCollisionCapsule::HitInfo ComponentCollisionCapsule::IsHit(ComponentCollisionPtr col)
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
        return isHit(std::dynamic_pointer_cast<ComponentCollisionCapsule>(shared_from_this()),
                     std::dynamic_pointer_cast<ComponentCollisionSphere>(col));
        break;

    case ComponentCollision::CollisionType::CAPSULE:
        return isHit(std::dynamic_pointer_cast<ComponentCollisionCapsule>(shared_from_this()),
                     std::dynamic_pointer_cast<ComponentCollisionCapsule>(col));
        break;
    case ComponentCollision::CollisionType::MODEL:
        return isHit(std::dynamic_pointer_cast<ComponentCollisionCapsule>(shared_from_this()),
                     std::dynamic_pointer_cast<ComponentCollisionModel>(col));
        break;
    }

    return info;
}

//! @brief ワールドMatrixの取得
//! @return 他のコンポーネントも含めた位置
const matrix ComponentCollisionCapsule::GetWorldMatrix()
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
