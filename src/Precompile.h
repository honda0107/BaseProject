// 必ずインクルードされるヘッダーと定義
// ※注意※ このヘッダーが参照されているファイル更新されるとフルビルドになるため
//          更新頻度が高いソースは追加しないこと。
#pragma once

//===========================================================================
// C++ STL
//===========================================================================

#include <cmath>
#include <fstream>
#include <algorithm>

//===========================================================================
// Windows SDK
//===========================================================================
#define NOMINMAX              // min/max マクロを無効化
#define WIN32_LEAN_AND_MEAN   // Win32で利用頻度が低いライブラリを除外して軽量化

// Windows10をサポート
#define WINVER       _WIN32_WINNT_WIN10
#define _WIN32_WINNT _WIN32_WINNT_WIN10

#include <wrl/client.h>   // Microsoft::WRL::ComPtr<T>　

//===========================================================================
// デバッグビルドでも強制最適化ONにするマクロ
//===========================================================================

#if defined(_DEBUG)
#define OPTIMIZE_DEBUG
#endif

#if defined(_MSC_VER) && defined(OPTIMIZE_DEBUG)

// デバッグ構成で、このコードセクションを強制的に最適化する。
#define DEBUG_OPTIMIZE_ON  __pragma(optimize("gt", on));   // 最適化ON
#define DEBUG_OPTIMIZE_OFF __pragma(optimize("", on));     // 最適化OFF

#else

#define DEBUG_OPTIMIZE_ON
#define DEBUG_OPTIMIZE_OFF

#endif

//===========================================================================
// 外部ライブラリ
//===========================================================================

// hlslpp
#include "hlslpp/include/hlsl++.h"
using namespace hlslpp;

// DirectX算術ライブラリ
#include <DirectXMath.h>

// DXライブラリ
#include "DxLib.h"

// Effekseer
#pragma warning(push)
#pragma warning(disable : 4100)   // 'value': 引数は関数の本体部で 1 度も参照されません。
#include "EffekseerForDXLib.h"
#pragma warning(pop)

//===========================================================================
// 実装コード
//===========================================================================

#include "System/Typedef.h"
#include "System/TypeInfo.h"
#include "System/VectorMath.h"
#include "System/ImGui.h"
#include "System/Input/InputKey.h"
#include "System/Input/InputPad.h"
#include "System/Input/InputMouse.h"
#include "System/Graphics/Render.h"
#include "System/Graphics/Shader.h"
#include "System/Graphics/Texture.h"
#include "System/Graphics/Model.h"

//--------------------------------------------------------------
// 数学定数
//--------------------------------------------------------------
static constexpr f32 PI       = 3.141592653589793f;   //!< 円周率 π
static constexpr f32 TAU      = 2.0f * PI;            //!< 円周率の2倍 τ
static constexpr f32 RadToDeg = 57.29577951f;         //!< Radian→Degree 変換係数
static constexpr f32 DegToRad = 0.017453293f;         //!< Degree→Radian 変換係数

//! Radian→Degree 単位変換
//! @param  [in]    radian  ラジアン値 (弧度法)
//! @return degree値 (度数法)
inline f32 R2D(f32 radian)
{
    return radian * RadToDeg;
}

//! Degree→Radian 単位変換
//! @param  [in]    degree  角度 (度数法)
//! @return radian値 (弧度法)
inline f32 D2R(f32 degree)
{
    return degree * DegToRad;
}

#define BLACK   GetColor(0, 0, 0)
#define WHITE   GetColor(255, 255, 255)
#define RED     GetColor(255, 0, 0)
#define GREEN   GetColor(0, 255, 0)
#define BLUE    GetColor(0, 0, 255)
#define YELLOW  GetColor(255, 255, 0)
#define PINK    GetColor(255, 0, 255)
#define SKYBLUE GetColor(0, 255, 255)

static constexpr s32 WINDOW_W = 1280;
static constexpr s32 WINDOW_H = 720;

// std::stringをstd::wstringへ変換する
std::wstring convertTo(std::string_view string);

// std::wstringをstd::stringへ変換する
std::string convertTo(std::wstring_view wstring);
