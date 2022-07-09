//---------------------------------------------------------------------------
//!	@file	InputPad.cpp
//! @brief	パッド入力管理
//---------------------------------------------------------------------------
#include "InputPad.h"

namespace
{
constexpr u32 MAX_PAD_NUM  = static_cast<u32>(PAD_ID::PAD_ID_MAX);   //<! 上下左右/A B C X Y Z L R START Mボタン
constexpr u32 MAX_PAD_TYPE = static_cast<u32>(PAD_NO::PAD_NO_MAX);   //<! PAD接続最大数
constexpr u32 PAD_BUTTONS[MAX_PAD_NUM] = {PAD_INPUT_UP,
                                          PAD_INPUT_DOWN,
                                          PAD_INPUT_LEFT,
                                          PAD_INPUT_RIGHT,

                                          PAD_INPUT_A,
                                          PAD_INPUT_B,
                                          PAD_INPUT_C,
                                          PAD_INPUT_X,
                                          PAD_INPUT_Y,
                                          PAD_INPUT_Z,
                                          PAD_INPUT_L,
                                          PAD_INPUT_R,
                                          PAD_INPUT_START,
                                          PAD_INPUT_M};
constexpr u32 PAD_TYPES[MAX_PAD_TYPE]  = {DX_INPUT_PAD1, DX_INPUT_PAD2, DX_INPUT_PAD3, DX_INPUT_PAD4};

u32 pads_[MAX_PAD_TYPE][MAX_PAD_NUM];
};   // namespace

//---------------------------------------------------------------------------
// 初期化
//---------------------------------------------------------------------------
void InputPadInit()
{
    for(int j = 0; j < MAX_PAD_TYPE; ++j) {
        for(int i = 0; i < MAX_PAD_NUM; ++i) {
            pads_[j][i] = 0;
        }
    }
}

//---------------------------------------------------------------------------
// 更新
//---------------------------------------------------------------------------
void InputPadUpdate()
{
    for(int j = 0; j < MAX_PAD_TYPE; ++j) {
        int tmp_pad = GetJoypadInputState(PAD_TYPES[j]);
        for(int i = 0; i < MAX_PAD_NUM; ++i) {
            if(tmp_pad & PAD_BUTTONS[i]) {
                ++pads_[j][i];
                if(pads_[j][i] > UINT_MAX)
                    pads_[j][i] = 1;
                continue;
            }

            pads_[j][i] = 0;
        }
    }
}

//---------------------------------------------------------------------------
// 終了
//---------------------------------------------------------------------------
void InputPadExit()
{
    ;
}

//---------------------------------------------------------------------------
// 特定パッドボタンの1回だけ押下検証
//---------------------------------------------------------------------------
bool IsPadOn(PAD_ID padID, PAD_NO pad_no)
{
    return (pads_[static_cast<u32>(pad_no)][static_cast<u32>(padID)] == 1);
}

//---------------------------------------------------------------------------
// 特定パッドボタンの解放検証
//---------------------------------------------------------------------------
bool IsPadRelease(PAD_ID padID, PAD_NO pad_no)
{
    return (pads_[static_cast<u32>(pad_no)][static_cast<u32>(padID)] == 0);
}

//---------------------------------------------------------------------------
// 特定パッドボタンの長押し検証
//---------------------------------------------------------------------------
bool IsPadRepeat(PAD_ID padID, PAD_NO pad_no)
{
    return (pads_[static_cast<u32>(pad_no)][static_cast<u32>(padID)] > 1);
}
