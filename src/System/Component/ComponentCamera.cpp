#include <System/Component/ComponentCamera.h>
#include <System/Component/ComponentTransform.h>
#include <System/Object.h>
#include <System/Scene.h>

#include <System/Debug/DebugCamera.h>
#include <System/ImGui.h>

ComponentCameraWeakPtr ComponentCamera::current_camera_{};

//---------------------------------------------------------
//! カメラ初期化
//---------------------------------------------------------
void ComponentCamera::Init()
{
    __super::Init();

    if(current_camera_.lock() == nullptr) {
        SetCurrentCamera();
    }

    //----------------------------------------------------------
    // 定数バッファを作成
    //----------------------------------------------------------
    cb_camera_info_ = CreateShaderConstantBuffer(sizeof(CameraInfo));

    // 標準ではPostUpdateでカメラは更新する
    SetUpdateTiming(ProcTiming::PostUpdate);

    camera_status_.on(CameraBit::Initialized);
}

//---------------------------------------------------------
//! カメラ更新
//---------------------------------------------------------
void ComponentCamera::PreUpdate()
{
    if(camera_status_.is(CameraBit::UpdateOnPreUpdate))
        SetCameraTransform();
}

//---------------------------------------------------------
//! カメラ更新
//---------------------------------------------------------
void ComponentCamera::Update([[maybe_unused]] float delta)
{
    if(camera_status_.is(CameraBit::UpdateOnUpdate))
        SetCameraTransform();
}

//---------------------------------------------------------
//! カメラ更新
//---------------------------------------------------------
void ComponentCamera::PostUpdate()
{
    if(camera_status_.is(CameraBit::UpdateOnPostUpdate))
        SetCameraTransform();

    //---------------------------------------------------------
    // 定数バッファを更新
    //---------------------------------------------------------
    // 更新に必要なメモリの場所を取得
    void* p = GetBufferShaderConstantBuffer(cb_camera_info_);
    {
        auto* info          = reinterpret_cast<CameraInfo*>(p);
        info->mat_view_     = mat_view_;
        info->mat_proj_     = mat_proj_;
        info->eye_position_ = position_;
    }

    // メモリをGPU側へ転送
    UpdateShaderConstantBuffer(cb_camera_info_);

    //---------------------------------------------------------
    // カメラ情報の定数バッファを設定
    //---------------------------------------------------------
    // b10 = CameraInfo
    SetShaderConstantBuffer(cb_camera_info_, DX_SHADERTYPE_VERTEX, 10);
    SetShaderConstantBuffer(cb_camera_info_, DX_SHADERTYPE_PIXEL, 10);
}

//----------------------------------------------------------
// カメラの設定
//----------------------------------------------------------
void ComponentCamera::SetCameraTransform()
{
    if(camera_status_.is(CameraBit::ChangeReq)) {
        auto current = current_camera_.lock();
        // 現状は瞬時に切り替える
        // TODO モーフィングなど
        if(current)
            current->camera_status_.off(CameraBit::Current);

        camera_status_.on(CameraBit::Current);
        camera_status_.off(CameraBit::ChangeReq);
        current_camera_ = std::weak_ptr<ComponentCamera>(
            std::dynamic_pointer_cast<ComponentCamera>(shared_from_this()));
    }

    //	if( !camera_status_.is( CameraBit::Current ) || ( DebugCamera::IsUse() && !camera_status_.is( CameraBit::DebugCameara ) ) )
    //		return;

    auto position = GetPosition();
    auto target   = GetTarget();

    if(!ImGuizmo::IsUsing()) {
        mat_view_ = matrix::lookAtLH(position, target);
        mat_proj_ = matrix::perspectiveFovLH(fovy_ * DegToRad, aspect_ratio_, near_z_, far_z_);
    }

    // デバッグカメラを使用してないとき、デバッグカメラで自分がデバッグカメラのときのみ有効とする
    if(!DebugCamera::IsUse() || (DebugCamera::IsUse() && camera_status_.is(CameraBit::DebugCameara))) {
        if(camera_status_.is(CameraBit::Current)) {
            SetCameraViewMatrix(mat_view_);            // ビュー行列
            SetupCamera_ProjectionMatrix(mat_proj_);   // 投影行列
        }
    }

    // 視錐台(Frustum)の更新
    if(camera_status_.is(CameraBit::Current)) {
        frustum_.setPosition(position);
        frustum_.setLookAt(target);
        frustum_.setWorldUp(up_);
        frustum_.setFov(fovy_ * DegToRad);
        frustum_.setDepthMode(Frustum::DepthMode::Default);
        frustum_.setAspectRatio(aspect_ratio_);

        frustum_.setNearZ(near_z_);
        frustum_.setFarZ(far_z_);
        frustum_.update();
    }
}

//! @brief カメラ位置の取得
//! @return カメラ位置

float3 ComponentCamera::GetPosition() const
{
    float4 position  = float4(position_, 1);
    auto   transform = GetOwner()->GetComponent<ComponentTransform>();
    if(transform)
        position = mul(position, transform->GetMatrix());

    return position.xyz;
}

//! @brief カメラターゲットの取得
//! @return カメラターゲット

float3 ComponentCamera::GetTarget() const
{
    float4 look_at   = float4(look_at_, 1);
    auto   transform = GetOwner()->GetComponent<ComponentTransform>();
    if(transform)
        look_at = mul(look_at, transform->GetMatrix());

    return look_at.xyz;
}

ComponentCamera::CameraRay ComponentCamera::MousePositionRay(int mouse_pos_x, int mouse_pos_y)
{
    VECTOR start = ConvScreenPosToWorldPos({(float)mouse_pos_x, (float)mouse_pos_y, 0.0f});
    VECTOR end   = ConvScreenPosToWorldPos({(float)mouse_pos_x, (float)mouse_pos_y, 1.0f});

    return CameraRay{cast(start), cast(end)};
}

//---------------------------------------------------------
//! 標準カメラや別のカメラから見た時のカメラモデルを表示する
//---------------------------------------------------------
void ComponentCamera::Draw()
{
    // 視錐台(Frustum)の更新
    if(camera_status_.is(CameraBit::ShowFrustum) ||
       DebugCamera::IsUse() && !camera_status_.is(CameraBit::DebugCameara)) {
        frustum_.renderDebug();
    }
}

//---------------------------------------------------------
//! カメラ終了処理
//---------------------------------------------------------
void ComponentCamera::Exit()
{
    // 定数バッファを解放
    DeleteShaderConstantBuffer(cb_camera_info_);

    __super::Exit();
}

//---------------------------------------------------------
//! カメラGUI処理
//---------------------------------------------------------
void ComponentCamera::GUI()
{
    assert(GetOwner());
    auto obj_name = GetOwner()->GetName();

    ImGui::Begin(obj_name.data());
    {
        ImGui::Separator();
        if(ImGui::TreeNode(u8"カメラステータス")) {
            u32* bit = &camera_status_.get();
            u32  val = *bit;
            ImGui::CheckboxFlags(u8"初期化済", &val, 1 << (int)CameraBit::Initialized);
            bool req = ImGui::CheckboxFlags(u8"カレントカメラ", &val, 1 << (int)CameraBit::Current);
            ImGui::CheckboxFlags(u8"タイムライン使用", bit, 1 << (int)CameraBit::EnableTimeLine);
            ImGui::CheckboxFlags(u8"カメラ【視錐台(Frustum)】を表示", bit, 1 << (int)CameraBit::ShowFrustum);

            if(req) {
                if((val & 1 << (int)CameraBit::Current) == 0) {
                    // カレントをやめる
                    camera_status_.off(CameraBit::Current);
                }
                else {
                    // カレントカメラにする
                    camera_status_.on(CameraBit::ChangeReq);
                }
            }

            ImGui::TreePop();
        }

        ImGui::DragFloat3(u8"カメラ座標", &position_.f32[0], 0.1f, -10000.0f, 10000.0f, "%.2f");
        ImGui::DragFloat3(u8"注視点", &look_at_.f32[0], 0.1f, -10000.0f, 10000.0f, "%.2f");
        ImGui::DragFloat3(u8"上ベクトル", &up_.f32[0], 0.1f, -10000.0f, 10000.0f, "%.2f");

        bool nf = false;
        nf |= ImGui::DragFloat(u8"Near", &near_z_, 0.01f, -10000.0f, 10000.0f, "%.2f");
        nf |= ImGui::DragFloat(u8"Far", &far_z_, 0.01f, -10000.0f, 10000.0f, "%.2f");
        if(nf)
            SetCameraNearFar(near_z_, far_z_);
    }
    ImGui::End();
}
