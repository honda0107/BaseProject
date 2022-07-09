//---------------------------------------------------------------------------
//! @file   SceneEaseCurve.cpp
//! @brief  Ease補間サンプルシーン
//---------------------------------------------------------------------------
#include "SceneEaseCurve.h"
#include "System/EaseCurve.h"
#include "System/SystemMain.h"   // ShowGrid

BP_CLASS_IMPL(SceneEaseCurve, u8"Ease補間サンプル")

//---------------------------------------------------------------------------
//! 初期化
//---------------------------------------------------------------------------
bool SceneEaseCurve::Init()
{
    // グリッド表示をOFF
    ShowGrid(false);

    return true;
}

//---------------------------------------------------------------------------
//! 更新
//! @param  [in]    delta   経過時間
//---------------------------------------------------------------------------
void SceneEaseCurve::Update(float delta)
{
    // 時間を経過させる
    t_ += delta * 0.5f;   // 1/2の速度

    // 0.0f～1.0fを繰り返し
    t_ = fmodf(t_, 1.0f);
}

//---------------------------------------------------------------------------
//! 描画
//---------------------------------------------------------------------------
void SceneEaseCurve::Draw()
{
    // Easeカーブ名
    static const char* easeName[]{
        "InSine",     "OutSine",      "InOutSine", "InQuad",     "OutQuad",       //
        "InOutQuad",  "InCubic",      "OutCubic",  "InOutCubic", "InQuart",       //
        "OutQuart",   "InOutQuart",   "InQuint",   "OutQuint",   "InOutQuint",    //
        "InExpo",     "OutExpo",      "InOutExpo", "InCirc",     "OutCirc",       //
        "InOutCirc",  "InBack",       "OutBack",   "InOutBack",  "InElastic",     //
        "OutElastic", "InOutElastic", "InBounce",  "OutBounce",  "InOutBounce",   //
    };

    constexpr u32 startX = 400;
    constexpr u32 endX   = 1000;

    // 補間係数tの位置にボックスで表示
    auto drawMark = [&](f32 t, u32 y, u32 color) {
        u32 boxSize = 16;

        u32 boxX = static_cast<u32>(lerp(float1(startX), float1(endX - boxSize), t));
        u32 boxY = y;
        DrawFillBox(boxX, boxY, boxX + boxSize, boxY + boxSize, color);
    };

    //----------------------------------------------------------
    // 画面クリア
    //----------------------------------------------------------
    ClearColor(float4(0.0f, 0.0f, 0.0f, 1.0f));

    //----------------------------------------------------------
    // 各Ease補間の結果を並べて表示
    //----------------------------------------------------------
    SetFontSize(18);   // 文字フォントの大きさ

    // スタートライン
    DrawLineAA(startX, 20, startX, 690, GetColor(0, 128, 255), 4.0f);

    // エンドライン
    DrawLineAA(endX, 20, endX, 690, GetColor(0, 128, 255), 4.0f);

    //----------------------------------------------------------
    // 線形補間の表示
    //----------------------------------------------------------
    {
        // 補間関数名を表示
        u32         y    = 80 + -2 * 20;
        const char* name = "LINEAR";

        u32  stringWidth = GetDrawStringWidth(name, static_cast<int>(strlen(name)));
        auto fontColor   = GetColor(255, 255, 255);

        DrawString(startX - 8 - stringWidth, y, name, fontColor);   // 左側 - 右詰め
        DrawString(endX + 8, y, name, fontColor);                   // 右側 - 左詰め

        // 補間後の位置をボックスで表示
        drawMark(t_, y, fontColor);
    }

    //----------------------------------------------------------
    // Ease補間の表示
    //----------------------------------------------------------
    for(u32 i = 0; i < GetEaseFunctionMaxCount(); ++i) {
        u32 type = i;   //補間関数の種類

        auto ease = GetEaseFunction(static_cast<EaseType>(type));

        // 補間関数名を表示
        u32  y           = 80 + i * 20;
        u32  stringWidth = GetDrawStringWidth(easeName[type], static_cast<int>(strlen(easeName[type])));
        bool isOdd       = i & 1;   // 1:奇数 0:偶数
        auto fontColor   = isOdd ? GetColor(255, 128, 128) : GetColor(128, 255, 128);

        DrawString(startX - 8 - stringWidth, y, easeName[type], fontColor);   // 左側 - 右詰め
        DrawString(endX + 8, y, easeName[type], fontColor);                   // 右側 - 左詰め

        // 補間後の位置をボックスで表示
        drawMark(ease(t_), y, fontColor);
    }
}

//---------------------------------------------------------------------------
//! 終了
//---------------------------------------------------------------------------
void SceneEaseCurve::Exit()
{
    // グリッド表示をON
    ShowGrid(true);
}

//---------------------------------------------------------------------------
//! GUI表示
//---------------------------------------------------------------------------
void SceneEaseCurve::GUI()
{
}
