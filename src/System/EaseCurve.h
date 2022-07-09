//---------------------------------------------------------------------------
//!	@file	EaseCurve.h
//! @brief	Easeカーブ補間
//---------------------------------------------------------------------------
#pragma once

//---------------------------------------------------------------------------
//! Easeカーブの種類
//---------------------------------------------------------------------------
enum class EaseType : u32
{
    InSine,
    OutSine,
    InOutSine,
    InQuad,
    OutQuad,
    InOutQuad,
    InCubic,
    OutCubic,
    InOutCubic,
    InQuart,
    OutQuart,
    InOutQuart,
    InQuint,
    OutQuint,
    InOutQuint,
    InExpo,
    OutExpo,
    InOutExpo,
    InCirc,
    OutCirc,
    InOutCirc,
    InBack,
    OutBack,
    InOutBack,
    InElastic,
    OutElastic,
    InOutElastic,
    InBounce,
    OutBounce,
    InOutBounce
};

//  Easeカーブ関数の種類の最大個数を取得
size_t GetEaseFunctionMaxCount();

//  Easeカーブ関数を取得
//! @param  [in]    type    カーブの種類
//! @return カーブ関数。std::function<f32(f32)>で受け取ります。
std::function<f32(f32)> GetEaseFunction(EaseType type);
