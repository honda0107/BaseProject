//---------------------------------------------------------------------------
//! @file   DebugCamera.h
//! @brief  デバッグカメラ
//---------------------------------------------------------------------------
#include "DebugCamera.h"
#include <System/Object.h>
#include <System/Input/InputMouse.h>
#include <System/Input/InputKey.h>
#include <System/Scene.h>

namespace DebugCamera
{
namespace
{
Control control = UnrealEngine;
bool    is_use  = false;
}   // namespace

// デバックカメラオブジェクト
class DebugCamera : public Object
{
public:
    bool Init() override
    {
        __super::Init();

        mat_view_ = GetCameraViewMatrix();
        mat_proj_ = GetCameraProjectionMatrix();

        matrix inv = inverse(mat_view_);
        cam_pos_   = inv.translate();
        tgt_pos_   = cam_pos_ + inv.axisZ() * 100.0f;

        auto cam = AddComponent<ComponentCamera>();
        cam->SetPositionAndTarget(cam_pos_, tgt_pos_);

        SetStatus(StatusBit::ShowGUI, false);
        return true;
    }

    void PreDraw() override
    {
        auto cam = GetComponent<ComponentCamera>();

        auto LR = (GetMouseInput() & (MOUSE_INPUT_LEFT | MOUSE_INPUT_RIGHT)) == (MOUSE_INPUT_LEFT | MOUSE_INPUT_RIGHT);
        auto M  = (GetMouseInput() & MOUSE_INPUT_MIDDLE);

        [[maybe_unused]] auto L = (GetMouseInput() & MOUSE_INPUT_LEFT);
        auto                  R = (GetMouseInput() & MOUSE_INPUT_RIGHT);

        float2 mouse_now = float2{GetMouseX(), GetMouseY()};
        mouse_vec_       = mouse_now - mouse_xy_;
        mouse_xy_        = mouse_now;

        // UnrealEngineのみ仮作成
        if(control == UnrealEngine) {
            // カメラに合わせた軸の作成
            float3 back  = normalize(cam_pos_ - tgt_pos_);
            float3 up    = {0, 1, 0};
            float3 right = cross(up, back);
            float3 front = -back;

            // wheel
            wheel_val = GetMouseWheelRotVolF();

            if(wheel_val != 0) {
                front *= wheel_val * 10;
                cam_pos_ = cam_pos_ + front;
                tgt_pos_ = tgt_pos_ + front;
            }

            // 平行移動
            if(M || LR) {
                float3 mv = (right * -mouse_vec_.x + up * -mouse_vec_.y) * 0.1f;

                cam_pos_ = cam_pos_ + mv;
                tgt_pos_ = tgt_pos_ + mv;
            }
            else if(R) {
                auto tgt_new_ = tgt_pos_ - cam_pos_;

                auto rot_y = matrix::rotateY(mouse_vec_.x * 0.003f);
                auto rot_x = matrix::rotateAxis(right, -mouse_vec_.y * 0.003f);

                tgt_pos_ = mul(float4(tgt_new_, 1), mul(rot_y, rot_x)).xyz + cam_pos_;
            }

            mat_view_ = matrix::lookAtLH(cam_pos_, tgt_pos_);

            cam->SetPositionAndTarget(cam_pos_, tgt_pos_);
            cam->SetCameraStatus(ComponentCamera::CameraBit::DebugCameara, true);

            SetCameraViewMatrix(mat_view_);            // ビュー行列
            SetupCamera_ProjectionMatrix(mat_proj_);   // 投影行列
        }
    }

private:
    float2 mouse_xy_  = {0, 0};
    float2 mouse_vec_ = {0, 0};
    float  wheel_val  = 0;

    float3 cam_pos_{};
    float3 tgt_pos_{};

    matrix mat_view_;
    matrix mat_proj_;
};
USING_PTR(DebugCamera);

//! @brief デバックカメラを使用する
//! @param use 使用するかどうか
void Use(bool use)
{
    // すでに設定済み
    if(is_use == use)
        return;

    is_use = use;

    if(use) {
        // デバッグカメラを作成します
        Scene::GetObjectPtrWithCreate<DebugCamera>()->SetName("DebugCamera");
    }
    else {
        // デバッグカメラを消去します
        if(auto obj = Scene::GetObjectPtr<DebugCamera>())
            Scene::ReleaseObject(obj);
    }
}

//! @brief デバッグカメラを使用中かどうか
//! @retval true : デバッグカメラ使用中
bool IsUse()
{
    return is_use;
}

//! @brief デバッグカメラの移動方法設定
//! @param mode 移動方式
void SetControl(Control mode)
{
    control = mode;
}

ComponentCameraWeakPtr GetCamera()
{
    auto cam      = Scene::GetObjectPtr<DebugCamera>("DebugCamera");
    auto weak_cam = std::weak_ptr<ComponentCamera>(cam->GetComponent<ComponentCamera>());

    return weak_cam;
}

}   // namespace DebugCamera
