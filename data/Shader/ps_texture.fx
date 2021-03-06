//----------------------------------------------------------------------------
//!	@file	ps_texture.fx
//!	@brief	テクスチャありピクセルシェーダー
//----------------------------------------------------------------------------
#include "dxlib_ps.h"

//----------------------------------------------------------------------------
// メイン関数
//----------------------------------------------------------------------------
PS_OUTPUT main(PS_INPUT input)
{
	PS_OUTPUT	output;

	// テクスチャカラーの読み込み
	float4	color = DiffuseTexture.Sample(DiffuseSampler, input.uv0_);

	// 出力カラー = テクスチャカラー * ディフューズカラー
	output.color0_ = color * input.diffuse_;

	// 出力パラメータを返す
	return output;
}
