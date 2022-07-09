//---------------------------------------------------------------------------
//! @file   VectorMath.cpp
//! @brief  ベクトル算術演算
//---------------------------------------------------------------------------
#include "VectorMath.h"

//---------------------------------------------------------------------------
//! 単位行列
//---------------------------------------------------------------------------
matrix matrix::identity()
{
    float4 m[4]{
        {1.0f, 0.0f, 0.0f, 0.0f},
        {0.0f, 1.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 1.0f, 0.0f},
        {0.0f, 0.0f, 0.0f, 1.0f}
    };
    return matrix(m[0], m[1], m[2], m[3]);
}

//---------------------------------------------------------------------------
//!  平行移動行列
//---------------------------------------------------------------------------
matrix matrix::translate(const float3& v)
{
    float4 m[4]{
        {1.0f, 0.0f, 0.0f, 0.0f},
        {0.0f, 1.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 1.0f, 0.0f},
        { v.x,  v.y,  v.z, 1.0f}
    };
    return matrix(m[0], m[1], m[2], m[3]);
}

matrix matrix::translate(f32 x, f32 y, f32 z)
{
    return translate(float3(x, y, z));
}

//---------------------------------------------------------------------------
//! スケール行列
//---------------------------------------------------------------------------
matrix matrix::scale(const float3& s)
{
    float4 m[4]{
        { s.x, 0.0f, 0.0f, 0.0f},
        {0.0f,  s.y, 0.0f, 0.0f},
        {0.0f, 0.0f,  s.z, 0.0f},
        {0.0f, 0.0f, 0.0f, 1.0f}
    };
    return matrix(m[0], m[1], m[2], m[3]);
}

matrix matrix::scale(f32 sx, f32 sy, f32 sz)
{
    return scale(float3(sx, sy, sz));
}

matrix matrix::scale(f32 s)
{
    return matrix::scale(float3(s, s, s));
}

//---------------------------------------------------------------------------
//! X軸中心の回転行列
//---------------------------------------------------------------------------
matrix matrix::rotateX(f32 radian)
{
    f32 s = std::sinf(radian);
    f32 c = std::cosf(radian);

    float4 m[4]{
        {1.0f, 0.0f, 0.0f, 0.0f},
        {0.0f,    c,    s, 0.0f},
        {0.0f,   -s,    c, 0.0f},
        {0.0f, 0.0f, 0.0f, 1.0f},
    };
    return matrix(m[0], m[1], m[2], m[3]);
}

//---------------------------------------------------------------------------
//! Y軸中心の回転行列
//---------------------------------------------------------------------------
matrix matrix::rotateY(f32 radian)
{
    f32 s = std::sinf(radian);
    f32 c = std::cosf(radian);

    float4 m[4]{
        {   c, 0.0f,   -s, 0.0f},
        {0.0f, 1.0f, 0.0f, 0.0f},
        {   s, 0.0f,    c, 0.0f},
        {0.0f, 0.0f, 0.0f, 1.0f}
    };
    return matrix(m[0], m[1], m[2], m[3]);
}

//---------------------------------------------------------------------------
//! Z軸中心の回転行列
//---------------------------------------------------------------------------
matrix matrix::rotateZ(f32 radian)
{
    f32 s = std::sinf(radian);
    f32 c = std::cosf(radian);

    float4 m[4]{
        {   c,    s, 0.0f, 0.0f},
        {  -s,    c, 0.0f, 0.0f},
        {0.0f, 0.0f, 1.0f, 0.0f},
        {0.0f, 0.0f, 0.0f, 1.0f}
    };
    return matrix(m[0], m[1], m[2], m[3]);
}

//---------------------------------------------------------------------------
//! 任意軸中心の回転行列
//---------------------------------------------------------------------------
matrix matrix::rotateAxis(const float3& axis, f32 radian)
{
    f32 s    = std::sinf(radian);
    f32 c    = std::cosf(radian);
    f32 invc = 1.0f - c;

    float3 v = normalize(axis);
    f32    x = v.x;
    f32    y = v.y;
    f32    z = v.z;

    float4 m[4]{
        {    c + x * x * invc, x * y * invc + z * s, z * x * invc - y * s, 0.0f},
        {x * y * invc - z * s,     c + y * y * invc, y * z * invc + x * s, 0.0f},
        {z * x * invc + y * s, y * z * invc - x * s,     c + z * z * invc, 0.0f},
        {                0.0f,                 0.0f,                 0.0f, 1.0f}
    };
    return matrix(m[0], m[1], m[2], m[3]);
}

//---------------------------------------------------------------------------
//!  [左手座標系] ビュー行列
//---------------------------------------------------------------------------
matrix matrix::lookAtLH(const float3& eye, const float3& lookAt, const float3& worldUp)
{
    float3 axis_z = normalize(lookAt - eye);
    float3 axis_x = normalize(cross(worldUp, axis_z));
    float3 axis_y = cross(axis_z, axis_x);

    f32 tx = -dot(axis_x, eye);
    f32 ty = -dot(axis_y, eye);
    f32 tz = -dot(axis_z, eye);

    float4 m[4]{
        {axis_x.x, axis_y.x, axis_z.x, 0.0f},
        {axis_x.y, axis_y.y, axis_z.y, 0.0f},
        {axis_x.z, axis_y.z, axis_z.z, 0.0f},
        {      tx,       ty,       tz, 1.0f},
    };
    return matrix(m[0], m[1], m[2], m[3]);
}

//---------------------------------------------------------------------------
//! [左手座標系] 投影行列
//---------------------------------------------------------------------------
matrix matrix::perspectiveFovLH(f32 fovy, f32 aspect_ratio, f32 near_z, f32 far_z)
{
    f32 s = std::sinf(fovy * 0.5f);
    f32 c = std::cosf(fovy * 0.5f);

    f32 height = c / s;
    f32 width  = height / aspect_ratio;
    f32 range  = far_z / (far_z - near_z);

    float4 m[4]{
        {width,   0.0f,            0.0f, 0.0f},
        { 0.0f, height,            0.0f, 0.0f},
        { 0.0f,   0.0f,           range, 1.0f},
        { 0.0f,   0.0f, -range * near_z, 0.0f}
    };
    return matrix(m[0], m[1], m[2], m[3]);
}

//---------------------------------------------------------------------------
//! [左手座標系] 無限遠投影行列
//---------------------------------------------------------------------------
matrix matrix::perspectiveFovInfiniteFarPlaneLH(f32 fovy, f32 aspect_ratio, f32 near_z)
{
    f32 s = std::sinf(fovy * 0.5f);
    f32 c = std::cosf(fovy * 0.5f);

    f32 height = c / s;
    f32 width  = height / aspect_ratio;

    float4 m[4]{
        {width,   0.0f,   0.0f, 0.0f},
        { 0.0f, height,   0.0f, 0.0f},
        { 0.0f,   0.0f,   0.0f, 1.0f},
        { 0.0f,   0.0f, near_z, 0.0f}
    };
    return matrix(m[0], m[1], m[2], m[3]);
}

//---------------------------------------------------------------------------
//! [左手座標系] 平行投影行列
//---------------------------------------------------------------------------
matrix matrix::orthographicOffCenterLH(f32 left, f32 right, f32 bottom, f32 top, f32 near_z, f32 far_z)
{
    float rcp_width  = 1.0f / (right - left);
    float rcp_height = 1.0f / (top - bottom);
    float range      = 1.0f / (far_z - near_z);

    float4 m[4]{
        {           rcp_width * 2.0f,                         0.0f,            0.0f, 0.0f},
        {                       0.0f,            rcp_height * 2.0f,            0.0f, 0.0f},
        {                       0.0f,                         0.0f,           range, 0.0f},
        {-(left + right) * rcp_width, -(top + bottom) * rcp_height, -range * near_z, 1.0f}
    };
    return matrix(m[0], m[1], m[2], m[3]);
}

//===========================================================================
//  要素ベクトル参照
//===========================================================================

//---------------------------------------------------------------------------
//! X軸ベクトルを取得
//---------------------------------------------------------------------------
float3 matrix::axisX() const
{
    return _11_12_13;
}

//---------------------------------------------------------------------------
//! Y軸ベクトルを取得
//---------------------------------------------------------------------------
float3 matrix::axisY() const
{
    return _21_22_23;
}

//---------------------------------------------------------------------------
//! Z軸ベクトルを取得
//---------------------------------------------------------------------------
float3 matrix::axisZ() const
{
    return _31_32_33;
}

//---------------------------------------------------------------------------
//! 平行移動を取得
//---------------------------------------------------------------------------
float3 matrix::translate() const
{
    return _41_42_43;
}
