//---------------------------------------------------------------------------
//! @file   Priority.h
//! @brief  優先設定
//---------------------------------------------------------------------------
#pragma once
#include <System/Signals.h>
#include <System/Cereal.h>

//---------------------------------------------------------------------------
//! プライオリティ(優先)
//---------------------------------------------------------------------------
enum class Priority
{
    NONE = -1,   //!< なし

    HIGHEST    = 100,     //!< 最優先
    HIGH       = 1000,    //!< 優先高
    NORMAL     = 10000,   //!< 通常
    LOW        = 15000,   //!< 優先低
    LOWEST     = 20000,   //!< 最低優先
    IFPOSSIBLE = 50000,   //!< 処理時間が無ければ処理しない

    NUM,
};

//! プライオリティ設定
constexpr int PRIORITY(Priority p)
{
    return static_cast<int>(p);
}

//! プライオリティ設定(Macro)
#define OBJ_PRIORITY(p) PRIORITY(Priority::##p)
