//---------------------------------------------------------------------------
//! @file   Geometry.h
//! @brief  ジオメトリ形状
//---------------------------------------------------------------------------
#pragma once

//===========================================================================
//! 光線 レイ
//===========================================================================
class Ray
{
public:
    float3 position_;   //!< 始点座標
    float3 dir_;        //!< レイの方向ベクトル

public:
    //! デフォルトコンストラクタ
    Ray() = default;

    //! コンストラクタ
    //! @param  [in]    position    始点座標
    //! @param  [in]    dir         レイの方向ベクトル
    Ray(const float3& position, const float3& dir)
        : position_(position)
        , dir_(dir)
    {
    }

    //! 座標を取得
    //! @param  [in]    t           パラメーターt
    //! @return パラメーターtで求められる座標
    float3 position(f32 t) const { return position_ + dir_ * t; }
};
