﻿//---------------------------------------------------------------------------
//! @file   InputKey.cpp
//! @brief  キー入力管理
//---------------------------------------------------------------------------
#include "InputKey.h"

namespace
{
constexpr int MAX_KEY_NUM = 256;
unsigned int  keys_[MAX_KEY_NUM];

//-----------------------------------------------------------------------
//! keys_配列検証用
//! @param  [in]    keyID   キー種別
//! @return キー種別が範囲外であればtrueが返ります。
//-----------------------------------------------------------------------
bool IsOverKeyNum(int keyID)
{
    return (keyID >= MAX_KEY_NUM) || (keyID < 0);
}
};   // namespace

//---------------------------------------------------------------------------
// 初期化
//---------------------------------------------------------------------------
void InputKeyInit()
{
    for(int i = 0; i < MAX_KEY_NUM; ++i) {
        keys_[i] = 0;
    }
}

//---------------------------------------------------------------------------
// 更新
//---------------------------------------------------------------------------
int InputKeyUpdate()
{
    char tmp_key[MAX_KEY_NUM];
    GetHitKeyStateAll(tmp_key);

    for(int i = 0; i < MAX_KEY_NUM; ++i) {
        if(tmp_key[i] != 0) {
            keys_[i]++;
            if(keys_[i] >= UCHAR_MAX)
                keys_[i] = UCHAR_MAX;
            continue;
        }

        keys_[i] = 0;
    }
    return 0;
}
//---------------------------------------------------------------------------
// 終了
//---------------------------------------------------------------------------
void InputKeyExit()
{
}

//---------------------------------------------------------------------------
// 指定キーの1回だけ押下検証
//---------------------------------------------------------------------------
bool IsKeyOn(int keyID)
{
    if(IsOverKeyNum(keyID))
        return false;
    return (keys_[keyID] == 1);
}

//---------------------------------------------------------------------------
// 指定キーが押されていない検証
//---------------------------------------------------------------------------
bool IsKeyRelease(int keyID)
{
    if(IsOverKeyNum(keyID))
        return false;
    return (keys_[keyID] == 0);
}

//---------------------------------------------------------------------------
// 指定キーの長押し検証
//---------------------------------------------------------------------------
bool IsKeyRepeat(int keyID)
{
    if(IsOverKeyNum(keyID))
        return false;
    return (keys_[keyID] > 1);
}
