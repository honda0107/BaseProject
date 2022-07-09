//---------------------------------------------------------------------------
//! @file   Frustum.h
//! @brief  視錐台
//---------------------------------------------------------------------------
#pragma once

//===========================================================================
//! カメラ
//===========================================================================
class Frustum
{
public:
    //! デプス動作モード
    enum class DepthMode : s32
    {
        Default,           //!< 通常
        Reverse,           //!< 反転Z
        ReverseInfinite,   //!< 反転Z (無限遠)
    };

    //----------------------------------------------------------
    //! @name   初期化
    //----------------------------------------------------------
    //@{

    //! デフォルトコンストラクタ
    Frustum() = default;

    //  コンストラクタ (行列を指定して初期化)
    Frustum(const matrix&      mat_view,
            const matrix&      mat_proj,
            Frustum::DepthMode depth_mode = Frustum::DepthMode::Default);

    //! デストラクタ
    virtual ~Frustum() = default;

    //@}
    //----------------------------------------------------------
    //! @name   更新
    //----------------------------------------------------------
    //@{

    //  更新
    void update();

    //  デバッグ描画
    void renderDebug();

    //@}
    //----------------------------------------------------------
    //! @name   設定
    //----------------------------------------------------------
    //@{

    //  位置を設定
    Frustum& setPosition(const float3& position);

    //  注視点を設定
    Frustum& setLookAt(const float3& look_at);

    //  世界の上方向のベクトルを設定
    Frustum& setWorldUp(const float3& world_up);

    //  上下左右の角度から注視点を設定
    //! @param  [in]    horizontal_angle    左右方向の水平回転角度
    //! @param  [in]    vertical_angle      上下方向の垂直回転角度
    //! @attention 位置からの相対方向から設定するため setPosition() の後に設定してください。
    Frustum& setLookAt(f32 horizontal_angle, f32 vertical_angle);

    //  画角を設定
    Frustum& setFov(f32 fovy);

    //  アスペクト比を設定
    Frustum& setAspectRatio(f32 aspect_ratio);

    //  近クリップ距離を設定
    Frustum& setNearZ(f32 z_near);

    //  遠クリップ距離を設定
    Frustum& setFarZ(f32 z_far);

    //  デプス動作モードを設定
    Frustum& setDepthMode(Frustum::DepthMode mode);

    //@}
    //----------------------------------------------------------
    //! @name   参照
    //----------------------------------------------------------
    //@{

    //  位置を取得
    [[nodiscard]] float3 position() const;

    //  注視点を取得
    [[nodiscard]] float3 lookAt() const;

    //  世界の上方向のベクトルを取得
    [[nodiscard]] float3 worldUp() const;

    //  画角を取得
    [[nodiscard]] f32 fov() const;

    //  アスペクト比を取得
    [[nodiscard]] f32 aspectRatio() const;

    //  近クリップ距離を取得
    [[nodiscard]] f32 nearZ() const;

    //  遠クリップ距離を取得
    [[nodiscard]] f32 farZ() const;

    //  スクリーンまでの距離を取得
    //! @param  [in]    resolution_height   画面解像度の高さ
    [[nodiscard]] f32 screenDistance(u32 resolution_height) const;

    //  デプス動作モードを取得
    [[nodiscard]] Frustum::DepthMode depthMode() const;

    //  反転Zを利用するかどうかを取得
    [[nodiscard]] bool useReverseDepth() const;

    //  スクリーン座標からレイを取得
    //! @param  [in]    screen_position スクリーン座標のXY値
    //! @return カメラ位置からのレイベクトル
    [[nodiscard]] float3 rayFromScreenPosition(float2 screen_position);

    //  カメラのワールド行列を取得
    [[nodiscard]] const matrix& matCameraWorld() const;

    //  ビュー行列を取得
    [[nodiscard]] const matrix& matView() const;

    //  投影行列を取得
    [[nodiscard]] const matrix& matProj() const;

    //  ビュー ✕ 投影行列を取得
    [[nodiscard]] const matrix& matViewProj() const;

    //@}

private:
    float3             position_         = float3(0.0f, 5.0f, -15.0f);    //!< 位置
    float3             look_at_          = float3(0.0f, 0.0f, 0.0f);      //!< 注視点
    float3             world_up_         = float3(0.0f, 1.0f, 0.0f);      //!< 世界の上方向のベクトル
    f32                fovy_             = 60.0f * DegToRad;              //!< 画角(単位:radian)
    f32                aspect_ratio_     = 16.0f / 9.0f;                  //!< アスペクト比
    f32                z_near_           = 0.01f;                         //!< 近クリップ面までの距離
    f32                z_far_            = 1000.0f;                       //!< 遠クリップ面までの距離
    Frustum::DepthMode depth_mode_       = Frustum::DepthMode::Default;   //!< デプス動作モード
    matrix             mat_camera_world_ = matrix::identity();            //!< カメラのワールド行列
    matrix             mat_view_         = matrix::identity();            //!< ビュー行列
    matrix             mat_proj_         = matrix::identity();            //!< 投影行列
    matrix             mat_view_proj_    = matrix::identity();            //!< ビュー ✕ 投影行列
};
