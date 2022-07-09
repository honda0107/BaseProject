//---------------------------------------------------------------------------
//! @file   Status.h
//! @brief  ステータス管理クラス
//---------------------------------------------------------------------------
#pragma once

//---------------------------------------------------------------------------
//! ステータス
//---------------------------------------------------------------------------
template <class T, class V = u32>
struct Status
{
    enum struct Bit : V
    {
        Alive = 0,      //!< 生存状態
        ChangePrio,     //!< プライオリティの変更中
        ShowGUI,        //!< GUI表示中
        Initialized,    //!< 初期化終了
        NoUpdate,       //!< Updateしない
        NoDraw,         //!< Drawしない
        DisablePause,   //!< ポーズ不可
        IsPause,        //!< ポーズ中
    };

    inline V on(T b)
    {
        status_bit_ |= 1ui64 << static_cast<V>(b);
        return status_bit_;
    }
    inline V off(T b)
    {
        status_bit_ &= ~(1ui64 << static_cast<V>(b));
        return status_bit_;
    }
    inline V  set(T b, bool _on) { return _on ? on(b) : off(b); }
    inline V  is(T b) const { return status_bit_ & (1ui64 << static_cast<int>(b)); }
    inline V& get() { return status_bit_; }

private:
    V status_bit_ = 0;
};
