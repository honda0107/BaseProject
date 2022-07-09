//---------------------------------------------------------------------------
//!	@file	EaseCurve.cpp
//! @brief	Easeカーブ補間
//---------------------------------------------------------------------------
#include "EaseCurve.h"

namespace
{

//---------------------------------------------------------------------------
f32 easeInSine(f32 t)
{
    return sinf(PI * 0.5f * t);
}

//---------------------------------------------------------------------------
f32 easeOutSine(f32 t)
{
    t -= 1.0f;
    return 1.0f + sinf(PI * 0.5f * t);
}

//---------------------------------------------------------------------------
f32 easeInOutSine(f32 t)
{
    return 0.5f * (1.0f + sinf(PI * (t - 0.5f)));
}

//---------------------------------------------------------------------------
f32 easeInQuad(f32 t)
{
    return t * t;
}

//---------------------------------------------------------------------------
f32 easeOutQuad(f32 t)
{
    return t * (2.0f - t);
}

//---------------------------------------------------------------------------
f32 easeInOutQuad(f32 t)
{
    return t < 0.5f ? 2.0f * t * t : t * (4.0f - 2.0f * t) - 1.0f;
}

//---------------------------------------------------------------------------
f32 easeInCubic(f32 t)
{
    return t * t * t;
}

//---------------------------------------------------------------------------
f32 easeOutCubic(f32 t)
{
    t -= 1.0f;
    return 1.0f + t * t * t;
}

//---------------------------------------------------------------------------
f32 easeInOutCubic(f32 t)
{
    return t < 0.5f ? 4.0f * t * t * t : 1.0f - powf(-2.0f * t + 2.0f, 3.0f) / 2.0f;
}

//---------------------------------------------------------------------------
f32 easeInQuart(f32 t)
{
    t *= t;
    return t * t;
}

//---------------------------------------------------------------------------
f32 easeOutQuart(f32 t)
{
    t -= 1.0f;
    t = t * t;
    return 1 - t * t;
}

//---------------------------------------------------------------------------
f32 easeInOutQuart(f32 t)
{
    if(t < 0.5f) {
        t *= t;
        return 8.0f * t * t;
    }
    else {
        t -= 1.0f;
        t = t * t;
        return 1.0f - 8.0f * t * t;
    }
}

//---------------------------------------------------------------------------
f32 easeInQuint(f32 t)
{
    f32 t2 = t * t;
    return t * t2 * t2;
}

//---------------------------------------------------------------------------
f32 easeOutQuint(f32 t)
{
    t -= 1.0f;
    f32 t2 = t * t;
    return 1.0f + t * t2 * t2;
}

//---------------------------------------------------------------------------
f32 easeInOutQuint(f32 t)
{
    f32 t2;
    if(t < 0.5f) {
        t2 = t * t;
        return 16.0f * t * t2 * t2;
    }
    else {
        t -= 1.0f;
        t2 = t * t;
        return 1.0f + 16.0f * t * t2 * t2;
    }
}

//---------------------------------------------------------------------------
f32 easeInExpo(f32 t)
{
    return (powf(2.0f, 8.0f * t) - 1.0f) / 255.0f;
}

//---------------------------------------------------------------------------
f32 easeOutExpo(f32 t)
{
    return 1.0f - powf(2.0f, -8.0f * t);
}

//---------------------------------------------------------------------------
f32 easeInOutExpo(f32 t)
{
    if(t < 0.5f) {
        return (powf(2.0f, 16.0f * t) - 1.0f) / 510.0f;
    }
    else {
        return 1.0f - 0.5f * powf(2.0f, -16.0f * (t - 0.5f));
    }
}

//---------------------------------------------------------------------------
f32 easeInCirc(f32 t)
{
    return 1.0f - sqrtf(1.0f - t);
}

//---------------------------------------------------------------------------
f32 easeOutCirc(f32 t)
{
    return sqrtf(t);
}

//---------------------------------------------------------------------------
f32 easeInOutCirc(f32 t)
{
    if(t < 0.5f) {
        return (1.0f - sqrtf(1.0f - 2.0f * t)) * 0.5f;
    }
    else {
        return (1.0f + sqrtf(2.0f * t - 1.0f)) * 0.5f;
    }
}

//---------------------------------------------------------------------------
f32 easeInBack(f32 t)
{
    return t * t * (2.70158f * t - 1.70158f);
}

//---------------------------------------------------------------------------
f32 easeOutBack(f32 t)
{
    t -= 1.0f;
    return 1.0f + t * t * (2.70158f * t + 1.70158f);
}

//---------------------------------------------------------------------------
f32 easeInOutBack(f32 t)
{
    if(t < 0.5f) {
        return t * t * (7.0f * t - 2.5f) * 2.0f;
    }
    else {
        t -= 1.0f;
        return 1.0f + t * t * 2.0f * (7.0f * t + 2.5f);
    }
}

//---------------------------------------------------------------------------
f32 easeInElastic(f32 t)
{
    f32 t2 = t * t;
    return t2 * t2 * sinf(t * PI * 4.5f);
}

//---------------------------------------------------------------------------
f32 easeOutElastic(f32 t)
{
    f32 t2 = (t - 1.0f) * (t - 1.0f);
    return 1.0f - t2 * t2 * cosf(t * PI * 4.5f);
}

//---------------------------------------------------------------------------
f32 easeInOutElastic(f32 t)
{
    f32 t2;
    if(t < 0.45f) {
        t2 = t * t;
        return 8.0f * t2 * t2 * sinf(t * PI * 9.0f);
    }
    else if(t < 0.55f) {
        return 0.5f + 0.75f * sinf(t * PI * 4.0f);
    }
    else {
        t2 = (t - 1.0f) * (t - 1.0f);
        return 1.0f - 8.0f * t2 * t2 * sinf(t * PI * 9.0f);
    }
}

//---------------------------------------------------------------------------
f32 easeInBounce(f32 t)
{
    return powf(2.0f, 6.0f * (t - 1.0f)) * fabsf(sinf(t * PI * 3.5f));
}

//---------------------------------------------------------------------------
f32 easeOutBounce(f32 t)
{
    return 1.0f - powf(2.0f, -6.0f * t) * fabsf(cosf(t * PI * 3.5f));
}

//---------------------------------------------------------------------------
f32 easeInOutBounce(f32 t)
{
    if(t < 0.5f) {
        return 8.0f * powf(2, 8.0f * (t - 1.0f)) * fabsf(sinf(t * PI * 7.0f));
    }
    else {
        return 1.0f - 8.0f * powf(2.0f, -8.0f * t) * fabsf(sinf(t * PI * 7.0f));
    }
}

}   // namespace

//---------------------------------------------------------------------------
//! Easeカーブ関数の種類の最大個数を取得
//---------------------------------------------------------------------------
size_t GetEaseFunctionMaxCount()
{
    return static_cast<size_t>(EaseType::InOutBounce) + 1;
}

//---------------------------------------------------------------------------
//! Easeカーブ関数を取得
//---------------------------------------------------------------------------
std::function<f32(f32)> GetEaseFunction(EaseType type)
{
    static std::function<f32(f32)> function[] = {
        easeInSine,    easeOutSine,    easeInOutSine,    easeInQuad,   easeOutQuad,   easeInOutQuad,
        easeInCubic,   easeOutCubic,   easeInOutCubic,   easeInQuart,  easeOutQuart,  easeInOutQuart,
        easeInQuint,   easeOutQuint,   easeInOutQuint,   easeInExpo,   easeOutExpo,   easeInOutExpo,
        easeInCirc,    easeOutCirc,    easeInOutCirc,    easeInBack,   easeOutBack,   easeInOutBack,
        easeInElastic, easeOutElastic, easeInOutElastic, easeInBounce, easeOutBounce, easeInOutBounce,
    };
    return function[static_cast<u32>(type)];
}
