//---------------------------------------------------------------------------
//! @file   ComponentCollisionModel.h
//! @brief  Sphereコリジョンコンポーネント
//---------------------------------------------------------------------------
#include <System/Component/ComponentCollisionModel.h>
#include <System/Component/ComponentCollisionSphere.h>
#include <System/Component/ComponentCollisionCapsule.h>
#include <System/Component/ComponentTransform.h>
#include <System/Component/ComponentModel.h>
#include <System/Object.h>
#include <System/Scene.h>

void ComponentCollisionModel::Init()
{
    __super::Init();
}

void ComponentCollisionModel::Update(float delta)
{
    __super::Update(delta);
}

// ここで当たり処理後の処理を行います
void ComponentCollisionModel::PostUpdate()
{
    __super::PostUpdate();
}

void ComponentCollisionModel::Draw()
{
    if(!Scene::IsEdit())
        return;

    __super::Draw();

    auto transform = GetWorldMatrix();

    SetUseLighting(FALSE);
    SetLightEnable(FALSE);

    // ポリゴンの数だけ繰り返し
    for(int i = 0; i < ref_poly_.PolygonNum; i++) {
        float3 p0 = cast(ref_poly_.Vertexs[ref_poly_.Polygons[i].VIndex[0]].Position);
        float3 p1 = cast(ref_poly_.Vertexs[ref_poly_.Polygons[i].VIndex[1]].Position);
        float3 p2 = cast(ref_poly_.Vertexs[ref_poly_.Polygons[i].VIndex[2]].Position);

        VECTOR v0 = cast(mul(float4(p0, 1), transform).xyz);
        VECTOR v1 = cast(mul(float4(p1, 1), transform).xyz);
        VECTOR v2 = cast(mul(float4(p2, 1), transform).xyz);

        // ポリゴンを形成する三頂点を使用してワイヤーフレームを描画する
        DrawLine3D(v0, v1, GetColor(255, 255, 0));

        DrawLine3D(v1, v2, GetColor(255, 255, 0));

        DrawLine3D(v2, v0, GetColor(255, 255, 0));
    }

    SetLightEnable(TRUE);
    SetUseLighting(TRUE);
}

void ComponentCollisionModel::Exit()
{
    __super::Exit();
}

//! @brief GUI処理
void ComponentCollisionModel::GUI()
{
    // 基底クラスのGUIはそのままは使用しない
    //__super::GUI();

    // オーナーの取得
    assert(GetOwner());
    auto obj_name = GetOwner()->GetName();

    ImGui::Begin(obj_name.data());
    {
        ImGui::Separator();
        auto ui_name = std::string("Collision Model:") + std::to_string(collision_id_);
        if(ImGui::TreeNode(ui_name.c_str())) {
            // コリジョン情報を出す
            GUICollsionData(false);

            std::string colname = u8"COL:" + std::to_string(collision_id_) + "/ ";

            float* mat = GetColMatrixFloat();
            float  matrixTranslation[3], matrixRotation[3], matrixScale[3];
            DecomposeMatrixToComponents(mat, matrixTranslation, matrixRotation, matrixScale);
            ImGui::DragFloat3((colname + u8"座標(T)").c_str(), matrixTranslation, 0.01f);
            //ImGui::DragFloat3( u8"COL回転(R)", matrixRotation, 0.1f );
            //ImGui::DragFloat3( u8"COLサイズ(S)", matrixScale, 0.01f );
            ImGuizmo::RecomposeMatrixFromComponents(matrixTranslation, matrixRotation, matrixScale, mat);

            ImGui::TreePop();
        }
    }
    ImGui::End();
}

void ComponentCollisionModel::AttachToModel()
{
    if(auto mdl = GetOwner()->GetComponent<ComponentModel>()) {
#ifdef USE_JOLT_PHYSICS
        // 地形衝突情報を作成 (メッシュ剛体は必ず静的)
        float3 scale = mdl->GetScaleAxisXYZ();
        bool   same  = (fabs(scale.x - scale.y) < FLT_EPSILON) && (fabs(scale.x - scale.z) < FLT_EPSILON);

        assert("マップのスケールのXYZが異なります。[無視する]で平均をとります." && same);
        float s     = (scale.x + scale.y + scale.z) / 3.0f;
        RigidBody() = physics::createRigidBody(shape::Mesh(mdl->GetModelClass().get(), s),   // メッシュ
                                               physics::ObjectLayers::NON_MOVING);           // 静的グループ
#else
        // 地形の傾きはDxLibからもらう
        MV1SetupReferenceMesh(mdl->GetModel(), -1, TRUE);
        ref_poly_ = MV1GetReferenceMesh(mdl->GetModel(), -1, TRUE);

        // ワールド行列を設定
        MV1SetMatrix(mdl->GetModel(), cast(mdl->GetWorldMatrix()));

        // モデルが存在すればその当たりを構築する(※Defaultを使用)
        MV1SetupCollInfo(mdl->GetModel());
#endif
    }
    else {
        assert(!"ComponentModelをAddしてから実行してください.");
    }
}

//! @brief 当たっているかを調べる
//! @param col 相手のコリジョン
//! @return HitInfoを返す
ComponentCollisionModel::HitInfo ComponentCollisionModel::IsHit(ComponentCollisionPtr col)
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
        return isHit(std::dynamic_pointer_cast<ComponentCollisionModel>(shared_from_this()),
                     std::dynamic_pointer_cast<ComponentCollisionSphere>(col));
        break;
    case ComponentCollision::CollisionType::CAPSULE:
        return isHit(std::dynamic_pointer_cast<ComponentCollisionModel>(shared_from_this()),
                     std::dynamic_pointer_cast<ComponentCollisionCapsule>(col));
        break;
    case ComponentCollision::CollisionType::MODEL:
        // 現状では当たらない
        break;
    }

    return info;
}

//! @brief ワールドMatrixの取得
//! @return 他のコンポーネントも含めた位置
const matrix ComponentCollisionModel::GetWorldMatrix()
{
    auto transform = matrix::identity();

    auto obj = GetOwner();
    {
        auto cmp = obj->GetComponent<ComponentTransform>();
        if(cmp) {
            transform = mul(transform, cmp->GetMatrix());
        }
        auto mdl = obj->GetComponent<ComponentModel>();
        if(mdl) {
            transform = mul(transform, mdl->GetMatrix());
        }
    }

    return transform;
}

//! @brief マップに対する正確な移動量を割り出す(傾きによる移動量)
//! @param vec 動きたい移動量
//! @param force 山に上る時の上りにくさ (1.0で普通に楽々上る ~ 3.0くらいだともう登れない )
//! @return 実際動ける量
float3 ComponentCollisionModel::checkMovement(float3 pos, float3 vec, float force)
{
    MV1_COLL_RESULT_POLY hit_poly{};
    float3               top = pos + float3(0, 10, 0);
    float3               btm = pos + float3(0, -1.5, 0);

    if(length(vec).x <= 0)
        return float3(0, 0, 0);

    if(auto mdl = GetOwner()->GetComponent<ComponentModel>()) {
        hit_poly = MV1CollCheck_Line(mdl->GetModel(), -1, cast(top), cast(btm));
        if(hit_poly.HitFlag) {
            // 制限あり (dotがマイナスということは山に上る形になっている)
            float pt = dot(cast(hit_poly.Normal), normalize(vec));
            if(pt < 0) {
                pt *= force;
                if(pt < -1)
                    pt = -1;

                // 傾きに合わせて、実際の移動量は、vec(最大) ~ 0(最小) となる
                return vec + vec * pt;
            }
        }
    }
    return vec;
}
