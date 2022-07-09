//---------------------------------------------------------------------------
//! @file   VectorMath.h
//! @brief  ベクトル算術演算
//---------------------------------------------------------------------------
#pragma once

//===========================================================================
//! @name   DxLib相互キャスト処理
//===========================================================================
//@{

//! DxLib::FLOAT2へキャスト
inline [[nodiscard]] DxLib::FLOAT2 cast(const float2& v)
{
    return {v.x, v.y};
}

//! DxLib::VECTOR/FLOAT3へキャスト
inline [[nodiscard]] DxLib::FLOAT3 cast(const float3& v)
{
    return {v.x, v.y, v.z};
}

//! DxLib::FLOAT4へキャスト
inline [[nodiscard]] DxLib::FLOAT4 cast(const float4& v)
{
    return {v.x, v.y, v.z, v.w};
}

//! DxLib::INT4へキャスト
inline [[nodiscard]] DxLib::INT4 cast(const int4& v)
{
    return {v.x, v.y, v.z, v.w};
}

//! DxLib::MATRIXへキャスト
inline [[nodiscard]] DxLib::MATRIX cast(const float4x4& m)
{
    DxLib::MATRIX result;
    store(m, reinterpret_cast<f32*>(&result));
    return result;
}

//! float2へキャスト
inline [[nodiscard]] float2 cast(const DxLib::FLOAT2& v)
{
    return {v.u, v.v};
}

//! float3へキャスト
inline [[nodiscard]] float3 cast(const DxLib::FLOAT3& v)
{
    return {v.x, v.y, v.z};
}

//! float4へキャスト
inline [[nodiscard]] float4 cast(const DxLib::FLOAT4& v)
{
    return {v.x, v.y, v.z, v.w};
}

//! int4へキャスト
inline [[nodiscard]] int4 cast(const DxLib::INT4& v)
{
    return {v.x, v.y, v.z, v.w};
}

//! float4x4へキャスト
inline [[nodiscard]] float4x4 cast(const DxLib::MATRIX& m)
{
    float4x4 result;
    load(result, reinterpret_cast<f32*>(const_cast<DxLib::MATRIX*>(&m)));
    return result;
}

//@}
//===========================================================================
//! @name   hlslppの追加関数
//===========================================================================
//@{

//! 3x4行列 ✕ float4
hlslpp_inline [[nodiscard]] float3 mul(const float3x4& m1, const float4& v)
{
    return float3(_hlslpp_mul_3x4_4x1_ps(m1.vec0, m1.vec1, m1.vec2, v.vec));
}

//@}
//===========================================================================
//! 4x4行列
//===========================================================================
class matrix : public float4x4
{
public:
    //! 継承コンストラクタ
    using float4x4::float4x4;

    //! コンストラクタ
    matrix(const float4x4& m)
        : float4x4(m)
    {
    }

    //----------------------------------------------------------
    //! @name   行列作成
    //----------------------------------------------------------
    //@{

    //! 単位行列
    static [[nodiscard]] matrix identity();

    // 平行移動行列
    //! @param  [in]    v   移動ベクトル
    static [[nodiscard]] matrix translate(const float3& v);
    static [[nodiscard]] matrix translate(f32 x, f32 y, f32 z);

    // スケール行列
    //! @param  [in]    s   スケール値
    static [[nodiscard]] matrix scale(const float3& s);
    static [[nodiscard]] matrix scale(f32 sx, f32 sy, f32 sz);
    static [[nodiscard]] matrix scale(f32 s);

    // X軸中心の回転行列
    //! @param  [in]    radian  回転角度
    //! @see https://ja.wikipedia.org/wiki/%E5%9B%9E%E8%BB%A2%E8%A1%8C%E5%88%97
    static [[nodiscard]] matrix rotateX(f32 radian);

    // Y軸中心の回転行列
    //! @param  [in]    radian  回転角度
    static [[nodiscard]] matrix rotateY(f32 radian);

    // Z軸中心の回転行列
    //! @param  [in]    radian  回転角度
    static [[nodiscard]] matrix rotateZ(f32 radian);

    // 任意軸中心の回転行列
    //! @param  [in]    axis    回転の中心軸
    //! @param  [in]    radian  回転角度
    static [[nodiscard]] matrix rotateAxis(const float3& axis, f32 radian);

    // [左手座標系] ビュー行列
    //! @param  [in]    eye         視点座標
    //! @param  [in]    look_at     注視点
    //! @param  [in]    world_up    世界の上方向のベクトル(default:(0.0f, 1.0f, 0.0f))
    static [[nodiscard]] matrix
    lookAtLH(const float3& eye, const float3& look_at, const float3& world_up = float3(0.0f, 1.0f, 0.0f));

    // [左手座標系] 投影行列
    //! @param  [in]    fovy            画角(単位:radian)
    //! @param  [in]    aspect_ratio    アスペクト比
    //! @param  [in]    near_z          近クリップZ値
    //! @param  [in]    far_z           遠クリップZ値
    //! @note InverseZにしたい場合はnearZの値とfarZの値を交換して指定。
    static [[nodiscard]] matrix perspectiveFovLH(f32 fovy, f32 aspect_ratio, f32 near_z, f32 far_z);

    // [左手座標系] 無限遠投影行列
    //!
    //! 遠クリップ面を廃止して無限遠まで描画可能にする行列。
    //! 浮動小数点の丸め誤差を低減。Z=0.0fで描画すると無限遠。
    //!
    //! @param  [in]    fovy         画角(単位:radian)
    //! @param  [in]    aspect_ratio アスペクト比
    //! @param  [in]    near_z       近クリップZ値
    //!
    //! @see GDC'07 「Projection Matrix Tricks」
    //! @attention InverseZ前提の投影にになるため注意。
    static [[nodiscard]] matrix perspectiveFovInfiniteFarPlaneLH(f32 fovy, f32 aspect_ratio, f32 near_z);

    // [左手座標系] 平行投影行列
    //! @param  [in]    left        左側の幅
    //! @param  [in]    right       右側の幅
    //! @param  [in]    bottom      下側の幅
    //! @param  [in]    top         上側の幅
    //! @param  [in]    near_z      近クリップZ値
    //! @param  [in]    far_z       遠クリップZ値
    //! @note InverseZにしたい場合はnearZの値とfarZの値を交換して指定。
    static [[nodiscard]] matrix
    orthographicOffCenterLH(f32 left, f32 right, f32 bottom, f32 top, f32 near_z, f32 far_z);

    //@}
    //----------------------------------------------------------
    //! @name   要素ベクトル参照
    //! 各要素へのアクセスはこのアクセサを利用します。
    //! float4で列ベクトルfloat4で直接操作を必要とする場合は
    //! 下記メンバーを利用してください
    //! - _11_12_13_14
    //! - _21_22_23_24
    //! - _31_32_33_34
    //! - _41_42_43_44
    //----------------------------------------------------------
    //@{

    // X軸ベクトルを取得
    float3 axisX() const;

    // Y軸ベクトルを取得
    float3 axisY() const;

    // Z軸ベクトルを取得
    float3 axisZ() const;

    // 平行移動を取得
    float3 translate() const;

    //! X軸ベクトル参照を取得
    auto& axisVectorX() { return _11_12_13_14; }

    //! Y軸ベクトル参照を取得
    auto& axisVectorY() { return _21_22_23_24; }

    //! Z軸ベクトル参照を取得
    auto& axisVectorZ() { return _31_32_33_34; }

    //! 平行移動ベクトル参照を取得
    auto& translateVector() { return _41_42_43_44; }

    //@}
    //----------------------------------------------------------
    //! @name   DxLib関連
    //----------------------------------------------------------
    //@{

    //! [DxLib] DxLib::MATRIXから初期化
    matrix(const DxLib::MATRIX& matrix) { load(*this, reinterpret_cast<f32*>(const_cast<DxLib::MATRIX*>(&matrix))); }

    //! [DxLib] DxLib::MATRIXへキャスト
    operator DxLib::MATRIX const() { return cast(*this); }

    //@}
};
