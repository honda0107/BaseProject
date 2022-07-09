//---------------------------------------------------------------------------
//! @file   Frustum.cpp
//! @brief  カメラ
//---------------------------------------------------------------------------
#include "Frustum.h"

//---------------------------------------------------------------------------
//! コンストラクタ (行列を指定して初期化)
//---------------------------------------------------------------------------
Frustum::Frustum(const matrix& mat_view, const matrix& mat_proj, Frustum::DepthMode depth_mode)
    : depth_mode_(depth_mode)
    , mat_view_(mat_view)
    , mat_proj_(mat_proj)
{
    // 各パラメーターを行列から抽出

    // float3            position_        = float3(0.0f, 5.0f, -15.0f);   //!< 位置
    // float3            look_at_         = float3(0.0f, 0.0f, 0.0f);     //!< 注視点
    // f32               fovy_            = 60.0f * DegToRad;             //!< 画角(単位:radian)
    // f32               aspect_ratio_    = 16.0f / 9.0f;                 //!< アスペクト比
    // f32               z_near_          = 0.01f;                        //!< 近クリップ面までの距離
    // f32               z_far_           = 1000.0f;                      //!< 遠クリップ面までの距離
    // Camera::DepthMode depthMode_       = Camera::DepthMode::Default;   //!< デプス動作モード
}

//---------------------------------------------------------------------------
//! 更新
//---------------------------------------------------------------------------
void Frustum::update()
{
    //----------------------------------------------------------
    // 行列を作成
    //----------------------------------------------------------
    // ビュー行列
    mat_view_ = matrix::lookAtLH(position_, look_at_, world_up_);

    // カメラのワールド行列はビュー行列の逆行列
    mat_camera_world_ = inverse(mat_view_);

    // 投影行列
    switch(depth_mode_) {
    case DepthMode::Default:
        mat_proj_ = matrix::perspectiveFovLH(fovy_, aspect_ratio_, z_near_, z_far_);
        break;
    case DepthMode::Reverse:
        // farとnearを入れ替えることでreverse-zになる
        mat_proj_ = matrix::perspectiveFovLH(fovy_, aspect_ratio_, z_far_, z_near_);
        break;
    case DepthMode::ReverseInfinite:
        // 無限遠まで高精度レンダリングする特殊投影行列
        mat_proj_ = matrix::perspectiveFovInfiniteFarPlaneLH(fovy_, aspect_ratio_, z_near_);
        break;
    }

    // 合成
    mat_view_proj_ = mul(mat_view_, mat_proj_);
}

//---------------------------------------------------------------------------
//! デバッグ描画
//---------------------------------------------------------------------------
void Frustum::renderDebug()
{
    f32 zf = 10.0f;   // 近クリップ距離
    f32 zn = 1.0f;    // 遠クリップ距離

    // nearとfarの距離の矩形の幅・高さ
    f32 nw = tanf(fovy_ * 0.5f) * zn * aspect_ratio_;
    f32 nh = tanf(fovy_ * 0.5f) * zn;
    f32 fw = tanf(fovy_ * 0.5f) * zf * aspect_ratio_;
    f32 fh = tanf(fovy_ * 0.5f) * zf;

    // カメラの姿勢情報
    float3 axis_x = mat_camera_world_.axisX();
    float3 axis_y = mat_camera_world_.axisY();
    float3 axis_z = mat_camera_world_.axisZ();

    //----------------------------------------------------------
    // 前後四隅の点を計算
    //----------------------------------------------------------
    float3 v[8];
    v[0] = position_ + axis_z * zn - axis_x * nw + axis_y * nh;
    v[1] = position_ + axis_z * zn + axis_x * nw + axis_y * nh;
    v[2] = position_ + axis_z * zn + axis_x * nw - axis_y * nh;
    v[3] = position_ + axis_z * zn - axis_x * nw - axis_y * nh;
    v[4] = position_ + axis_z * zf - axis_x * fw + axis_y * fh;
    v[5] = position_ + axis_z * zf + axis_x * fw + axis_y * fh;
    v[6] = position_ + axis_z * zf + axis_x * fw - axis_y * fh;
    v[7] = position_ + axis_z * zf - axis_x * fw - axis_y * fh;

    SetUseLighting(false);

    //----------------------------------------------------------
    // デプス無効でシルエットを描画
    //----------------------------------------------------------
    SetUseZBuffer3D(false);
    {
        DrawLine3D(cast(v[0]), cast(v[1]), GetColor(0, 0, 0));
        DrawLine3D(cast(v[1]), cast(v[2]), GetColor(0, 0, 0));
        DrawLine3D(cast(v[2]), cast(v[3]), GetColor(0, 0, 0));
        DrawLine3D(cast(v[3]), cast(v[0]), GetColor(0, 0, 0));

        DrawLine3D(cast(v[4]), cast(v[5]), GetColor(0, 0, 0));
        DrawLine3D(cast(v[5]), cast(v[6]), GetColor(0, 0, 0));
        DrawLine3D(cast(v[6]), cast(v[7]), GetColor(0, 0, 0));
        DrawLine3D(cast(v[7]), cast(v[4]), GetColor(0, 0, 0));

        DrawLine3D(cast(v[0]), cast(v[4]), GetColor(0, 0, 0));
        DrawLine3D(cast(v[1]), cast(v[5]), GetColor(0, 0, 0));
        DrawLine3D(cast(v[2]), cast(v[6]), GetColor(0, 0, 0));
        DrawLine3D(cast(v[3]), cast(v[7]), GetColor(0, 0, 0));

        DrawLine3D(cast(position_ + axis_z * zn), cast(position_ + axis_z * zf * 2.0f), GetColor(0, 0, 0));
    }

    for(u32 i = 0; i < 8; ++i) {
        DrawSphere3D(cast(v[i]), 0.1f, 8, GetColor(255, 255, 0), 0, true);
    }

    //----------------------------------------------------------
    // デプス有効でシルエットを描画
    //----------------------------------------------------------
    SetUseZBuffer3D(true);
    {
        DrawLine3D(cast(v[0]), cast(v[1]), GetColor(0, 255, 0));
        DrawLine3D(cast(v[1]), cast(v[2]), GetColor(0, 255, 0));
        DrawLine3D(cast(v[2]), cast(v[3]), GetColor(0, 255, 0));
        DrawLine3D(cast(v[3]), cast(v[0]), GetColor(0, 255, 0));

        DrawLine3D(cast(v[4]), cast(v[5]), GetColor(0, 255, 0));
        DrawLine3D(cast(v[5]), cast(v[6]), GetColor(0, 255, 0));
        DrawLine3D(cast(v[6]), cast(v[7]), GetColor(0, 255, 0));
        DrawLine3D(cast(v[7]), cast(v[4]), GetColor(0, 255, 0));

        DrawLine3D(cast(v[0]), cast(v[4]), GetColor(0, 255, 0));
        DrawLine3D(cast(v[1]), cast(v[5]), GetColor(0, 255, 0));
        DrawLine3D(cast(v[2]), cast(v[6]), GetColor(0, 255, 0));
        DrawLine3D(cast(v[3]), cast(v[7]), GetColor(0, 255, 0));

        DrawLine3D(cast(position_ + axis_z * zn), cast(position_ + axis_z * zf * 2.0f), GetColor(0, 255, 0));
    }
    SetUseLighting(true);
}

//---------------------------------------------------------------------------
//! 位置を設定
//---------------------------------------------------------------------------
Frustum& Frustum::setPosition(const float3& position)
{
    position_ = position;
    return *this;
}

//---------------------------------------------------------------------------
//! 注視点を設定
//---------------------------------------------------------------------------
Frustum& Frustum::setLookAt(const float3& look_at)
{
    look_at_ = look_at;
    return *this;
}

//---------------------------------------------------------------------------
//! 世界の上方向のベクトルを設定
//---------------------------------------------------------------------------
Frustum& Frustum::setWorldUp(const float3& world_up)
{
    world_up_ = world_up;
    return *this;
}

//---------------------------------------------------------------------------
//! 上下左右の角度から注視点を設定
//---------------------------------------------------------------------------
Frustum& Frustum::setLookAt(f32 horizontal_angle, f32 vertical_angle)
{
    matrix m = mul(matrix::rotateX(vertical_angle), matrix::rotateY(horizontal_angle));

    look_at_ = position_ + m.axisZ();

    return *this;
}

//---------------------------------------------------------------------------
//! 画角を設定
//---------------------------------------------------------------------------
Frustum& Frustum::setFov(f32 fovy)
{
    fovy_ = fovy;
    return *this;
}

//---------------------------------------------------------------------------
//! アスペクト比を設定
//---------------------------------------------------------------------------
Frustum& Frustum::setAspectRatio(f32 aspect_ratio)
{
    aspect_ratio_ = aspect_ratio;
    return *this;
}

//---------------------------------------------------------------------------
//! 近クリップ距離を設定
//---------------------------------------------------------------------------
Frustum& Frustum::setNearZ(f32 z_near)
{
    z_near_ = z_near;
    return *this;
}

//---------------------------------------------------------------------------
//! 遠クリップ距離を設定
//---------------------------------------------------------------------------
Frustum& Frustum::setFarZ(f32 z_far)
{
    z_far = z_far;
    return *this;
}

//---------------------------------------------------------------------------
//! デプス動作モードを設定
//---------------------------------------------------------------------------
Frustum& Frustum::setDepthMode(Frustum::DepthMode mode)
{
    depth_mode_ = mode;

    return *this;
}

//---------------------------------------------------------------------------
//! 位置を取得
//---------------------------------------------------------------------------
float3 Frustum::position() const
{
    return position_;
}

//---------------------------------------------------------------------------
//! 注視点を取得
//---------------------------------------------------------------------------
float3 Frustum::lookAt() const
{
    return look_at_;
}

//---------------------------------------------------------------------------
//! 世界の上方向のベクトルを取得
//---------------------------------------------------------------------------
float3 Frustum::worldUp() const
{
    return world_up_;
}

//---------------------------------------------------------------------------
//! 画角を取得
//---------------------------------------------------------------------------
f32 Frustum::fov() const
{
    return fovy_;
}

//---------------------------------------------------------------------------
//! アスペクト比を取得
//---------------------------------------------------------------------------
f32 Frustum::aspectRatio() const
{
    return aspect_ratio_;
}

//---------------------------------------------------------------------------
//! 近クリップ距離を取得
//---------------------------------------------------------------------------
f32 Frustum::nearZ() const
{
    return z_near_;
}

//---------------------------------------------------------------------------
//! 遠クリップ距離を取得
//---------------------------------------------------------------------------
f32 Frustum::farZ() const
{
    return z_far_;
}

//---------------------------------------------------------------------------
//! スクリーンまでの距離を取得
//---------------------------------------------------------------------------
f32 Frustum::screenDistance(u32 resolution_height) const
{
    //                  +
    //              ／  |
    //           ／     |
    //        ／        | resolution_height /2
    //     ／           |
    //  ／）fovy_ / 2   |
    //+-----------------+
    //          d  スクリーンまでの距離
    //
    // tan(fovy_ / 2) = (resolution_height / 2) / d
    //
    return static_cast<f32>(resolution_height / 2) / tanf(fovy_ * 0.5f);
}

//---------------------------------------------------------------------------
//! デプス動作モードを取得
//---------------------------------------------------------------------------
Frustum::DepthMode Frustum::depthMode() const
{
    return depth_mode_;
}

//---------------------------------------------------------------------------
//! 反転Zを利用するかどうかを取得
//---------------------------------------------------------------------------
bool Frustum::useReverseDepth() const
{
    return (depth_mode_ == DepthMode::Reverse) || (depth_mode_ == DepthMode::ReverseInfinite);
}

//---------------------------------------------------------------------------
//! スクリーン座標からレイを取得
//---------------------------------------------------------------------------
float3 Frustum::rayFromScreenPosition(float2 screen_position)
{
    // カメラの姿勢情報
    float3 axis_x = mat_camera_world_.axisX();
    float3 axis_y = mat_camera_world_.axisY();
    float3 axis_z = mat_camera_world_.axisZ();

    return screen_position.x * axis_x + screen_position.y / aspect_ratio_ * axis_y + axis_z;
}

//---------------------------------------------------------------------------
//! カメラのワールド行列を取得
//---------------------------------------------------------------------------
const matrix& Frustum::matCameraWorld() const
{
    return mat_camera_world_;
}

//---------------------------------------------------------------------------
//! ビュー行列を取得
//---------------------------------------------------------------------------
const matrix& Frustum::matView() const
{
    return mat_view_;
}

//---------------------------------------------------------------------------
//! 投影行列を取得
//---------------------------------------------------------------------------
const matrix& Frustum::matProj() const
{
    return mat_proj_;
}

//---------------------------------------------------------------------------
//! ビュー ✕ 投影行列を取得
//---------------------------------------------------------------------------
const matrix& Frustum::matViewProj() const
{
    return mat_view_proj_;
}
