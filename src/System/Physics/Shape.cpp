//---------------------------------------------------------------------------
//!	@file	Shape.cpp
//! @brief	形状
//---------------------------------------------------------------------------
#include "Shape.h"
#include "System/Graphics/Model.h"
#include "System/Graphics/ResourceModel.h"
#include "System/Graphics/ModelCache.h"

namespace shape
{

//===========================================================================
//! 凸形状 ConvexHull
//===========================================================================

//---------------------------------------------------------------------------
//! コンストラクタ
//---------------------------------------------------------------------------
ConvexHull::ConvexHull(Model* model)
    : shape::Base(shape::Type::ConvexHull)
{
    auto* resource_model = model->resource();
    auto* model_cache    = resource_model->modelCache();

    // リソースがロード中の場合は読み込みが終わるまで待つ
    resource_model->waitForReadFinish();

    //----------------------------------------------------------
    // モデルキャッシュから頂点配列とインデックス配列を取得
    // この配列は既にリダクションされたジオメトリです
    //----------------------------------------------------------
    // 頂点配列
    auto& varray = model_cache->vertices();
    for(auto& v : varray) {
        vertices_.push_back(cast(v));   // DxLib::VECTOR→float3にキャストしながらコピー
    }
}

//===========================================================================
//! メッシュ Mesh
//===========================================================================

//---------------------------------------------------------------------------
//! コンストラクタ
//---------------------------------------------------------------------------
Mesh::Mesh(Model* model, f32 scale)
    : shape::Base(shape::Type::Mesh)
{
    auto* resource_model = model->resource();
    auto* model_cache    = resource_model->modelCache();

    // モデルキャッシュが有効ではない場合は読み込みが終わるまで待つ
    if(model_cache->isValid() == false) {
        resource_model->waitForReadFinish();
    }

    //----------------------------------------------------------
    // モデルキャッシュから頂点配列とインデックス配列を取得
    // この配列は既にリダクションされたジオメトリです
    //----------------------------------------------------------
    // 頂点配列
    auto& varray = model_cache->vertices();
    for(auto& v : varray) {
        vertices_.push_back(cast(v) * scale);   // DxLib::VECTOR→float3にキャストしながらコピー
    }

    // インデックス配列
    indices_ = model_cache->indices();
}

}   // namespace shape
