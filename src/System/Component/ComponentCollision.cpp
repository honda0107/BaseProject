//---------------------------------------------------------------------------
//! @file   ComponentCollision.h
//! @brief  コリジョンコンポーネント(ベースクラス)
//---------------------------------------------------------------------------
#include <System/Component/ComponentCollision.h>
#include <System/Component/ComponentTransform.h>

#include <System/Component/ComponentCollisionCapsule.h>
#include <System/Component/ComponentCollisionSphere.h>
#include <System/Component/ComponentCollisionModel.h>
#include <System/Component/ComponentModel.h>

#include <System/Object.h>

ComponentCollision::ComponentCollision(ObjectPtr owner)
    : Component(owner)
{
    // 複数設定可能とする
    SetStatus(Component::StatusBit::SameType, true);

    // 識別子設定
    assert(GetOwner());
    auto cmps = GetOwner()->GetComponents<ComponentCollision>();
    int  max  = -1;
    for(ComponentCollisionPtr cmp : cmps) {
        if((int)cmp->collision_id_ > max)
            max = (int)cmp->collision_id_;
    }
    collision_id_ = (u32)(max + 1);
}

//! @brief 当たった情報はコールバックで送られてくる
//! @param hitInfo 当たった情報
//! @details 当たった回数分ここに来ます
void ComponentCollision::OnHit(const HitInfo& hitInfo)
{
    auto obj = GetOwner();

    // Staticな物質にぶつかった場合、gravity_を下げる
    if(hitInfo.hit_collision_->GetMass() < 0) {
        auto   vec = obj->GetTranslate() - obj->GetOldWorldMatrix().translate();
        float3 nvc = {0.0f, -1.0f, 0.0f};
        if(length(vec).x <= 0 || length(now_gravity_).x <= 0) {
            now_gravity_ = 0.0f;
        }
        else {
            nvc     = normalize(vec);
            float d = dot(normalize(now_gravity_), nvc);
            now_gravity_ *= (1 - d * d * 0.1f);
        }
    }

    obj->OnHit(hitInfo);
}

void ComponentCollision::GUICollsionData(bool use_attach)
{
    auto str = u8"コリジョンタイプ : " + CollisionTypeName[(u32)collision_type_];
    ImGui::Text(str.c_str());
    ImGui::Separator();
    // @todo ListBox化する
    str = u8"コリジョングループ : " + GetCollisionGroupName();
    ImGui::Text(str.c_str());
    ImGui::Separator();
    //  ImGui::CheckboxFlags(u8"初期化済み", &collision_status_.get(), 1 << (u32)CollisionBit::Initialized);
    ImGui::CheckboxFlags(u8"ヒットしない", &collision_status_.get(), 1 << (u32)CollisionBit::DisableHit);
    ImGui::CheckboxFlags(u8"ゲーム中表示", &collision_status_.get(), 1 << (u32)CollisionBit::ShowInGame);
    ImGui::Separator();
    ImGui::Text(u8"Hitするグループ");
    ImGui::CheckboxFlags("WALL", (u32*)&collision_hit_, (u32)CollisionGroup::WALL);
    ImGui::CheckboxFlags("GROUND", (u32*)&collision_hit_, (u32)CollisionGroup::GROUND);
    ImGui::CheckboxFlags("PLAYER", (u32*)&collision_hit_, (u32)CollisionGroup::PLAYER);
    ImGui::CheckboxFlags("ENEMY", (u32*)&collision_hit_, (u32)CollisionGroup::ENEMY);
    ImGui::CheckboxFlags("WEAPON", (u32*)&collision_hit_, (u32)CollisionGroup::WEAPON);
    ImGui::CheckboxFlags("ITEM", (u32*)&collision_hit_, (u32)CollisionGroup::ITEM);
    ImGui::CheckboxFlags("CAMERA", (u32*)&collision_hit_, (u32)CollisionGroup::CAMERA);
    ImGui::CheckboxFlags("ETC", (u32*)&collision_hit_, (u32)CollisionGroup::ETC);
    ImGui::Separator();
    ImGui::Checkbox(u8"重力を使用する", &use_gravity_);
    ImGui::DragFloat3(u8"重力加速度", (float*)&gravity_);

    // アタッチが存在しないときはここで終了
    if(!use_attach)
        return;

    if(auto cmp = GetOwner()->GetComponent<ComponentModel>()) {
        bool attach = false;
        if(attach_node_ >= 0) {
            attach = true;
            if(ImGui::Checkbox("AttachNode", &attach)) {
                if(!attach)
                    attach_node_ = -1;
            }
        }
        else {
            attach = false;
            if(ImGui::Checkbox("AttachNode", &attach)) {
                if(attach)
                    attach_node_ = 0;
            }
        }

        auto items = cmp->GetNodesNamePChar();
        if(ImGui::Combo("Node", &attach_node_, items.data(), (int)items.size())) {
            // 切り替えたとき
            collision_transform_ = matrix::identity();
        }
    }
}

void ComponentCollision::Update(float delta)
{
    __super::Update(delta);
    if(attach_node_ >= 0) {
        attach_node_matrix_ = matrix::identity();
        if(auto mdl = GetOwner()->GetComponent<ComponentModel>()) {
            attach_node_matrix_ = MV1GetFrameLocalWorldMatrix(mdl->GetModel(), attach_node_);
        }
    }
#ifdef USE_JOLT_PHYSICS
#else
    // 重力加速度
    if(use_gravity_) {
        GetOwner()->SetGravity(now_gravity_);
        now_gravity_ += gravity_ * delta;
    }
#endif
}

void ComponentCollision::PostUpdate()
{
    //! 古いマトリクスを更新します
    old_transform_ = collision_transform_;
}

//! @brief GUI処理
void ComponentCollision::GUI()
{
}

void ComponentCollision::AttachToModel(int node)
{
    if(auto mdl = GetOwner()->GetComponent<ComponentModel>()) {
        attach_node_ = node;
#ifdef USE_JOLT_PHYSICS
        if(GetRigidBody())
            GetRigidBody()->setGravityFactor(0.0f);
#endif USE_JOLT_PHYSICS
    }
}

//! @brief Capsule VS Sphere
//! @param col1 Capsuleコリジョン
//! @param col2 Sphere コリジョン
//! @return 当たり情報
ComponentCollision::HitInfo ComponentCollision::isHit(ComponentCollisionCapsulePtr col1,
                                                      ComponentCollisionSpherePtr  col2)
{
    ComponentCollision::HitInfo info{};

    // 自分のコリジョン
    float3 cpos1 = col1->GetTranslate();
    float3 cpos2 = col1->GetVectorAxisY() * col1->GetHeight() + cpos1;
    float  cs    = 1.0f;   //< スケール

    // モデルアタッチ
    if(col1->attach_node_ >= 0) {
        if(auto mdl = col1->GetOwner()->GetComponent<ComponentModel>()) {
            cpos1 = mul(float4(cpos1, 1), col1->attach_node_matrix_).xyz;
            cpos2 = mul(float4(cpos2, 1), col1->attach_node_matrix_).xyz;
            cpos2 = normalize(cpos2 - cpos1) * col1->GetHeight() + cpos1;
        }
    }
    else {
        // ComponentTransform(オブジェクト姿勢)
        if(auto cmp = col1->GetOwner()->GetComponent<ComponentTransform>()) {
            // 高さに回転とスケールを掛け合わせる
            cpos1 = mul(float4(cpos1, 1), cmp->GetMatrix()).xyz;
            cpos2 = mul(float4(cpos2, 1), cmp->GetMatrix()).xyz;
            // 半径はXZで平均としておく
            cs = (length(cmp->GetMatrix().axisX()) + length(cmp->GetMatrix().axisZ())) / 2;
        }
    }

    float3 epos1;
    float  es = 1.0f;

    // モデルアタッチ
    if(col2->attach_node_ >= 0) {
        if(auto mdl = col2->GetOwner()->GetComponent<ComponentModel>()) {
            epos1 = col2->GetTranslate();
            epos1 = mul(float4(epos1, 1), col2->attach_node_matrix_).xyz;
        }
    }
    else {
        if(auto cmp = col2->GetOwner()->GetComponent<ComponentTransform>()) {
            epos1 = mul(col2->GetMatrix(), cmp->GetMatrix())._41_42_43;
            //pos1 = mul( float4( pos1, 0 ) , cmp->GetMatrix() ).xyz;
            //pos1 += cmp->GetTranslate().xyz;
            float sx = length(cmp->GetVectorAxisX());
            float sy = length(cmp->GetVectorAxisY());
            float sz = length(cmp->GetVectorAxisZ());
            es       = (sx + sy + sz) / 3.0f;
        }
    }

    float  cr = col1->GetRadius() * cs;
    float3 cv = normalize(cpos1 - cpos2);
    VECTOR c1 = cast(cpos1 - cv * cr);
    VECTOR c2 = cast(cpos2 + cv * cr);

    float  er = col2->GetRadius() * es;
    VECTOR e1 = cast(epos1);

    // 跳ね返り点が欲しいため、HitCheck_Capsule_Capsuleは使わない
    SEGMENT_POINT_RESULT result;
    Segment_Point_Analyse(&c1, &c2, &e1, &result);

    if(result.Seg_Point_MinDist_Square < (cr + er) * (cr + er)) {
        // 線と線で一番近くなる点を求め、ベクトル化する
        // 最も近い点
        float3 c0  = cast(result.Seg_MinDist_Pos);
        float3 e0  = cast(e1);
        float3 vec = e0 - c0;   // 調べたほうの跳ね返りの方向(100%)
        float  len = length(vec);
        if(abs(len) <= abs(len) * FLT_EPSILON) {
            // 全く同じ位置にいる場合はz移動する形にしておく
            vec = {0, 0, 1};
        }

        float3 vs = normalize(vec) * (cr + er);
        vec -= vs;

        // このpush_は、調べたほうの押し戻し方向100%で作成する
        info.push_         = vec;
        info.hit_          = true;
        info.hit_position_ = (e0 + c0) * 0.5f;
    }

    return info;
}

//! @brief Sphere VS Capsule
//! @param col1 Sphereコリジョン
//! @param col2 Capsule コリジョン
//! @return 当たり情報
ComponentCollision::HitInfo ComponentCollision::isHit(ComponentCollisionSpherePtr  col1,
                                                      ComponentCollisionCapsulePtr col2)
{
    auto hit  = isHit(col2, col1);
    hit.push_ = -hit.push_;   // push方向を反対にする
    return hit;
}

//! @brief Capsule VS Capsule
//! @param col1 Capsuleコリジョン
//! @param col2 Capsule コリジョン
//! @return 当たり情報
ComponentCollision::HitInfo ComponentCollision::isHit(ComponentCollisionCapsulePtr col1,
                                                      ComponentCollisionCapsulePtr col2)
{
    ComponentCollision::HitInfo info{};

    // 自分のコリジョン
    float3 cpos1 = col1->GetTranslate();
    float3 cpos2 = normalize(col1->GetVectorAxisY()) * col1->GetHeight() + cpos1;
    float  cs    = 1.0f;   //< スケール

    // モデルアタッチ
    if(col1->attach_node_ >= 0) {
        if(auto mdl = col1->GetOwner()->GetComponent<ComponentModel>()) {
            cpos1 = mul(float4(cpos1, 1), col1->attach_node_matrix_).xyz;
            cpos2 = mul(float4(cpos2, 1), col1->attach_node_matrix_).xyz;
            cpos2 = normalize(cpos2 - cpos1) * col1->GetHeight() + cpos1;
        }
    }
    else {
        // ComponentTransform(オブジェクト姿勢)
        if(auto cmp = col1->GetOwner()->GetComponent<ComponentTransform>()) {
            auto& mtx = cmp->GetMatrix();
            // 高さに回転とスケールを掛け合わせる
            cpos1 = mul(float4(cpos1, 1), mtx).xyz;
            cpos2 = mul(float4(cpos2, 1), mtx).xyz;
            // 半径はXZで平均としておく
            cs = (length(mtx.axisX()) + length(mtx.axisZ())) / 2;
        }
    }

    // 相手のコリジョン
    float3 epos1 = col2->GetTranslate();
    float3 epos2 = normalize(col2->GetVectorAxisY()) * col2->GetHeight() + epos1;
    float  es    = 1.0f;   //< スケール

    // モデルアタッチ
    if(col2->attach_node_ >= 0) {
        if(auto mdl = col2->GetOwner()->GetComponent<ComponentModel>()) {
            epos1 = mul(float4(epos1, 1), col2->attach_node_matrix_).xyz;
            epos2 = mul(float4(epos2, 1), col2->attach_node_matrix_).xyz;
            epos2 = normalize(epos2 - epos1) * col2->GetHeight() + epos1;
        }
    }
    else {
        // ComponentTransform(オブジェクト姿勢)
        if(auto cmp = col2->GetOwner()->GetComponent<ComponentTransform>()) {
            auto& mtx = cmp->GetMatrix();
            // 高さに回転とスケールを掛け合わせる
            epos1 = mul(float4(epos1, 1), mtx).xyz;
            epos2 = mul(float4(epos2, 1), mtx).xyz;
            // 半径はXZで平均としておく
            es = (length(mtx.axisX()) + length(mtx.axisZ())) / 2;
        }
    }

    float  cr = col1->GetRadius() * cs;
    float3 cv = normalize(cpos1 - cpos2);
    VECTOR c1 = cast(cpos1 - (cv * cr));
    VECTOR c2 = cast(cpos2 + (cv * cr));
    float  er = col2->GetRadius() * es;
    float3 ev = normalize(epos1 - epos2);
    VECTOR e1 = cast(epos1 - (ev * er));
    VECTOR e2 = cast(epos2 + (ev * er));

    // 跳ね返り点が欲しいため、HitCheck_Capsule_Capsuleは使わない
    SEGMENT_SEGMENT_RESULT result;
    Segment_Segment_Analyse(&c1, &c2, &e1, &e2, &result);

#if 0   // 当たり不具合チェック用
	if ( col1->attach_node_ )
		DrawCapsule3D( c1, c2, cr, 10, GetColor( 0, 0, 255 ), GetColor( 0, 0, 255 ), FALSE );

	if ( col2->attach_node_ )
		DrawCapsule3D( e1, e2, er, 10, GetColor( 0, 0, 255 ), GetColor( 0, 0, 255 ), FALSE );
#endif

    if(result.SegA_SegB_MinDist_Square < (cr + er) * (cr + er)) {
        // 線と線で一番近くなる点を求め、ベクトル化する
        // 最も近い点
        float3 c0  = cast(result.SegA_MinDist_Pos);
        float3 e0  = cast(result.SegB_MinDist_Pos);
        float3 vec = e0 - c0;   // 調べたほうの跳ね返りの方向(100%)

        float len = length(vec);
        if(abs(len) <= abs(len) * FLT_EPSILON) {
            // 全く同じ位置にいる場合はz移動する形にしておく
            vec = {0, 0, 1};
        }
        float3 vs = normalize(vec) * (cr + er);
        vec -= vs;

        // このpush_は、調べたほうの押し戻し方向100%で作成する
        info.push_         = vec;
        info.hit_          = true;
        info.hit_position_ = (e0 + c0) * 0.5f;
    }

    return info;
}

//! @brief Sphere VS Sphere
//! @param col1 Sphereコリジョン
//! @param col2 Sphere コリジョン
//! @return 当たり情報
ComponentCollision::HitInfo ComponentCollision::isHit(ComponentCollisionSpherePtr col1,
                                                      ComponentCollisionSpherePtr col2)
{
    ComponentCollision::HitInfo info{};

    float3 pos1;
    float  scale1 = 1.0f;

    // モデルアタッチ
    if(col1->attach_node_ >= 0) {
        if(auto mdl = col1->GetOwner()->GetComponent<ComponentModel>()) {
            pos1 = col1->GetTranslate();
            pos1 = mul(float4(pos1, 1), col1->attach_node_matrix_).xyz;
        }
    }
    else {
        if(auto cmp = col1->GetOwner()->GetComponent<ComponentTransform>()) {
            pos1 = mul(col1->GetMatrix(), cmp->GetMatrix())._41_42_43;
            //pos1 = mul( float4( pos1, 0 ) , cmp->GetMatrix() ).xyz;
            //pos1 += cmp->GetTranslate().xyz;
            float sx = length(cmp->GetVectorAxisX());
            float sy = length(cmp->GetVectorAxisY());
            float sz = length(cmp->GetVectorAxisZ());
            scale1   = (sx + sy + sz) / 3.0f;
        }
    }

    float3 pos2;
    float  scale2 = 1.0f;

    // モデルアタッチ
    if(col2->attach_node_ >= 0) {
        if(auto mdl = col2->GetOwner()->GetComponent<ComponentModel>()) {
            pos2 = col2->GetTranslate();
            pos2 = mul(float4(pos2, 1), col2->attach_node_matrix_).xyz;
        }
    }
    else {
        if(auto cmp = col2->GetOwner()->GetComponent<ComponentTransform>()) {
            pos2 = mul(col2->GetMatrix(), cmp->GetMatrix())._41_42_43;
            //pos2 = mul( float4( pos2, 0 ), cmp->GetMatrix() ).xyz;
            //pos2 += cmp->GetTranslate().xyz;
            float sx = length(cmp->GetVectorAxisX());
            float sy = length(cmp->GetVectorAxisY());
            float sz = length(cmp->GetVectorAxisZ());
            scale2   = (sx + sy + sz) / 3.0f;
        }
    }

    if(HitCheck_Sphere_Sphere(cast(pos1), col1->GetRadius() * scale1, cast(pos2), col2->GetRadius() * scale2)) {
        // 中間地点を当たった場所にする
        info.hit_          = true;
        info.hit_position_ = (pos1 - pos2) * 0.5f + pos2;

        // 押し出し方向
        float3 distance = pos1 - pos2;
        float  len      = length(distance);
        if(abs(len) <= abs(len) * FLT_EPSILON) {
            // 全く同じ位置にいる場合はz移動する形にしておく
            distance = {0, 0, 1};
        }

        float3 vec     = normalize(distance);
        float  reallen = col1->GetRadius() * scale1 + col2->GetRadius() * scale2;
        vec            = vec * (reallen - len);

        // このpush_は、調べたほうの押し戻し方向100%で作成する
        info.push_ = vec;
    }

    return info;
}

//! @brief Model VS Sphere
//! @param col1 Model コリジョン
//! @param col2 Sphereコリジョン
//! @return 当たり情報
ComponentCollision::HitInfo ComponentCollision::isHit(ComponentCollisionSpherePtr col1, ComponentCollisionModelPtr col2)
{
    ComponentCollision::HitInfo info{};

    const float no_clumb = 0.5f;   // これ以下の傾きは上らない

    // モデルが存在していない
    auto mdl = col2->GetOwner()->GetComponent<ComponentModel>();
    if(mdl == nullptr)
        return info;

    float3 opos{};
    float3 cpos{};
    if(col1->attach_node_ >= 0) {
        cpos = mul(float4(col1->GetTranslate(), 1), col1->attach_node_matrix_).xyz;
    }
    else {
        // オブジェクト位置に対するコリジョン(1フレーム前)
        opos = mul(float4(col1->GetTranslate(), 1), col1->GetOwner()->GetOldWorldMatrix()).xyz;

        // オブジェクト位置に対するコリジョン
        cpos = mul(float4(col1->GetTranslate(), 1), col1->GetOwner()->GetMatrix()).xyz;

        // 実際の移動できる量にする
        auto  move  = cpos - opos;
        float movey = move.y;
        move.y      = 0;

        if(col1->IsUseGravity()) {
            move = col2->checkMovement(opos, move, 1.5f);
            //move += col1->now_gravity_;
            move.y = movey;
            cpos   = opos + move;

            // 当たりからキャラの位置を求める
            auto rot                      = col1->GetOwner()->GetMatrix();
            rot._41_42_43                 = {0, 0, 0};
            auto col1r                    = mul(float4(col1->GetTranslate(), 1), rot).xyz;
            col1->GetOwner()->Translate() = cpos - col1r;
        }
    }

    float scale = 1.0f;
    if(col1->attach_node_ < 0) {
        auto  mat = col1->GetMatrix();
        float sx  = length(mat.axisVectorX());
        float sy  = length(mat.axisVectorY());
        float sz  = length(mat.axisVectorZ());
        scale     = (sx + sy + sz) / 3.0f;
    }

    cpos       = cpos + float3{0, -col1->GetRadius() * scale, 0};
    auto cpos1 = opos;   //cpos + float3{ 0, col1->GetRadius() * scale * 2, 0 };

    float3 pos = cpos;

    MV1_COLL_RESULT_POLY hit_poly{};
    float3               bottom = cpos;    //-float3{ 0, col1->GetRadius() * scale, 0 };
    float3               top    = cpos1;   //bottom + float3{ 0, col1->GetRadius() * scale * 2, 0 };
    hit_poly                    = MV1CollCheck_Line(mdl->GetModel(), -1, cast(top), cast(bottom));
    //DrawLine3D( cast( top ), cast( bottom ), GetColor( 255, 0, 255 ) );

    if(hit_poly.HitFlag != 0) {
        float d = dot(cast(hit_poly.Normal), float3(0, 1, 0));
        //float e = 1.0f;
        if(abs(d) > no_clumb)   //< 傾き過ぎているとかなり上るので抑える
        {
            pos = cast(hit_poly.HitPosition);

            // 上下に球にめり込む分を移動させる
            // ※傾きが大きいと移動も大きいため許容するほうが楽
            //e = 1.0f / d;
            //pos.y += ( e - 1 ) * col1->GetRadius() * scale;

            // 半径分押し戻し
            float3 vec = pos - cpos;

            // 一旦1つ目の当たりだけで返してみる
            // このpush_は、調べたほうの押し戻し方向100%で作成する
            info.push_         = vec;
            info.hit_          = true;
            info.hit_position_ = pos;
        }
    }

    {
        // 次は壁当たり球の当たりをチェックする
        MV1_COLL_RESULT_POLY_DIM hit_poly_dim{};

        float3 vec_up = normalize(cpos1 - cpos);
        //float  vec_h  = length( cpos1 - cpos );

        float3 pos2 = pos + vec_up * col1->GetRadius() * scale;
        float3 pos1 = cpos1 - vec_up * col1->GetRadius() * scale;

        // Capsuleの処理を使っているため面倒なのでこれで代用しておく
        hit_poly_dim = MV1CollCheck_Capsule(mdl->GetModel(), -1, cast(pos1), cast(pos2), col1->GetRadius() * scale);
        float3 vh    = 0;
        for(int i = 0; i < hit_poly_dim.HitNum; i++) {
            if(dot(cast(hit_poly_dim.Dim[i].Normal), float3(0, 1, 0)).x > 0.5f)
                continue;

            SEGMENT_TRIANGLE_RESULT result{};

            VECTOR v1 = cast(pos1);
            VECTOR v2 = cast(pos2);
            Segment_Triangle_Analyse(&v1,
                                     &v2,
                                     &hit_poly_dim.Dim[i].Position[0],
                                     &hit_poly_dim.Dim[i].Position[1],
                                     &hit_poly_dim.Dim[i].Position[2],
                                     &result);

            float3 add = cast(result.Tri_MinDist_Pos);

            add.y = pos.y;
            //result.Seg_MinDist_Pos.y;

            //add.y	  = pos.y;
            float len = (col1->GetRadius() * scale) - length(pos - add);
            add       = pos - add;
            if(len > 0)
                vh += add * len;

            info.hit_ = true;
        }
        pos += vh;

        // 一旦1つ目の当たりだけで返してみる
        // このpush_は、調べたほうの押し戻し方向100%で作成する
        info.push_         = pos - cpos;
        info.hit_position_ = pos;
    }
    return info;
}

//! @brief Sphere VS Capsule
//! @param col1 Sphereコリジョン
//! @param col2 Capsule コリジョン
//! @return 当たり情報
ComponentCollision::HitInfo ComponentCollision::isHit(ComponentCollisionModelPtr col1, ComponentCollisionSpherePtr col2)
{
    auto hit  = isHit(col2, col1);
    hit.push_ = -hit.push_;   // push方向を反対にする
    return hit;
}

//! @brief Model VS Capsule
//! @param col1 Modelコリジョン
//! @param col2 Capsule コリジョン
//! @return 当たり情報
ComponentCollision::HitInfo ComponentCollision::isHit(ComponentCollisionCapsulePtr col1,
                                                      ComponentCollisionModelPtr   col2)
{
    ComponentCollision::HitInfo info{};

    const float no_clumb = 0.5f;   // これ以下の傾きは上らない

    // モデルが存在していない
    auto mdl = col2->GetOwner()->GetComponent<ComponentModel>();
    if(mdl == nullptr)
        return info;

    float3 opos{};
    float3 cpos{};
    if(col1->attach_node_ >= 0) {
        cpos = mul(float4(col1->GetTranslate(), 1), col1->attach_node_matrix_).xyz;
    }
    else {
        // オブジェクト位置に対するコリジョン(1フレーム前)
        opos = mul(float4(col1->GetTranslate(), 1), col1->GetOwner()->GetOldWorldMatrix()).xyz;

        // オブジェクト位置に対するコリジョン
        cpos = mul(float4(col1->GetTranslate(), 1), col1->GetOwner()->GetMatrix()).xyz;

        // 実際の移動できる量にする
        auto  move  = cpos - opos;
        float movey = move.y;
        move.y      = 0;

        if(col1->IsUseGravity()) {
            move = col2->checkMovement(opos, move, 1.5f);
            //move += col1->now_gravity_;
            move.y = movey;
            cpos   = opos + move;

            // 当たりからキャラの位置を求める
            auto rot                      = col1->GetOwner()->GetMatrix();
            rot._41_42_43                 = {0, 0, 0};
            auto col1r                    = mul(float4(col1->GetTranslate(), 1), rot).xyz;
            col1->GetOwner()->Translate() = cpos - col1r;
        }
    }

    // その頭の位置
    float3 cpos1 = mul(float4(col1->GetTranslate() + float3(0, col1->GetHeight(), 0), 1),
                       col1->GetOwner()->GetOldWorldMatrix())
                       .xyz;

    // 高さ
    //float height = length( cpos - cpos1 );

    float scale = 1.0f;
    if(col1->attach_node_ < 0) {
        auto  mat = col1->GetMatrix();
        float sx  = length(mat.axisVectorX());
        float sy  = length(mat.axisVectorY());
        float sz  = length(mat.axisVectorZ());
        scale     = (sx + sy + sz) / 3.0f;
    }

    float3 pos = cpos;

    MV1_COLL_RESULT_POLY hit_poly{};
    float3               bottom = cpos;    //-float3{ 0, col1->GetRadius() * scale, 0 };
    float3               top    = cpos1;   //bottom + float3{ 0, col1->GetRadius() * scale * 2, 0 };
    hit_poly                    = MV1CollCheck_Line(mdl->GetModel(), -1, cast(top), cast(bottom));
    //DrawLine3D( cast( top ), cast( bottom ), GetColor( 255, 0, 255 ) );

    if(hit_poly.HitFlag != 0) {
        float d = dot(cast(hit_poly.Normal), float3(0, 1, 0));
        //float e = 1.0f;
        if(abs(d) > no_clumb)   //< 傾き過ぎているとかなり上るので抑える
        {
            pos = cast(hit_poly.HitPosition);

            // 上下に球にめり込む分を移動させる
            // ※傾きが大きいと移動も大きいため許容するほうが楽

            //e = 1.0f / d;
            //pos.y += ( e - 1 ) * col1->GetRadius() * scale;

            // 半径分押し戻し
            float3 vec = pos - cpos;

            // 一旦1つ目の当たりだけで返してみる
            // このpush_は、調べたほうの押し戻し方向100%で作成する
            info.push_         = vec;
            info.hit_          = true;
            info.hit_position_ = pos;
        }
    }

    {
        // 次は壁当たり球の当たりをチェックする
        MV1_COLL_RESULT_POLY_DIM hit_poly_dim{};

        float3 vec_up = normalize(cpos1 - cpos);
        //float  vec_h  = length( cpos1 - cpos );

        float3 pos2 = pos + vec_up * col1->GetRadius() * scale;
        float3 pos1 = cpos1 - vec_up * col1->GetRadius() * scale;

        hit_poly_dim = MV1CollCheck_Capsule(mdl->GetModel(), -1, cast(pos1), cast(pos2), col1->GetRadius() * scale);
        float3 vh    = 0;
        for(int i = 0; i < hit_poly_dim.HitNum; i++) {
            if(dot(cast(hit_poly_dim.Dim[i].Normal), float3(0, 1, 0)).x > 0.5f)
                continue;

            SEGMENT_TRIANGLE_RESULT result{};

            VECTOR v1 = cast(pos1);
            VECTOR v2 = cast(pos2);
            Segment_Triangle_Analyse(&v1,
                                     &v2,
                                     &hit_poly_dim.Dim[i].Position[0],
                                     &hit_poly_dim.Dim[i].Position[1],
                                     &hit_poly_dim.Dim[i].Position[2],
                                     &result);

            float3 add = cast(result.Tri_MinDist_Pos);

            add.y = pos.y;
            //result.Seg_MinDist_Pos.y;

            //add.y	  = pos.y;
            float len = (col1->GetRadius() * scale) - length(pos - add);
            add       = pos - add;
            if(len > 0)
                vh += add * len;

            info.hit_ = true;
        }
        pos += vh;

        // 一旦1つ目の当たりだけで返してみる
        // このpush_は、調べたほうの押し戻し方向100%で作成する
        info.push_         = pos - cpos;
        info.hit_position_ = pos;
    }
    return info;
}

//! @brief Capsule VS Model
//! @param col1 Capsuleコリジョン
//! @param col2 Model コリジョン
//! @return 当たり情報
ComponentCollision::HitInfo ComponentCollision::isHit(ComponentCollisionModelPtr   col1,
                                                      ComponentCollisionCapsulePtr col2)
{
    auto hit  = isHit(col2, col1);
    hit.push_ = -hit.push_;   // push方向を反対にする
    return hit;
}
