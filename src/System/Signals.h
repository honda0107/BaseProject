//---------------------------------------------------------------------------
//! @file   Scene.h
//! @brief  シーン
//---------------------------------------------------------------------------
#pragma once

#include <sigslot/signal.hpp>

//---------------------------------------------------------------------------
// シグナル
//---------------------------------------------------------------------------
using SignalsUpdate  = sigslot::signal<float>;
using SignalsDraw    = sigslot::signal<>;
using SignalsPrePost = sigslot::signal<>;
using SignalsGUI     = sigslot::signal<>;

// ユーザーシグナル
using SignalsShadow  = sigslot::signal<>;
using SignalsGbuffer = sigslot::signal<>;
using SignalsLight   = sigslot::signal<>;
using SignalsHDR     = sigslot::signal<>;
