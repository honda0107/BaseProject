//---------------------------------------------------------------------------
//! @file   ProcTiming.h
//! @brief  処理タイミング
//---------------------------------------------------------------------------
#pragma once
#include "Priority.h"
#include <System/Signals.h>
#include <System/Cereal.h>

//! タイミング
enum struct ProcTiming : u32
{
    PreUpdate = 0,
    Update,
    LateUpdate,
    PrePhysics,
    PostUpdate,

    PreDraw,
    Draw,
    LateDraw,
    PostDraw,

    Shadow,
    Gbuffer,
    Light,
    HDR,

    NUM,
};

//! プライオリティ設定
constexpr int TIMING(ProcTiming p)
{
    return static_cast<int>(p);
}

//! プライオリティ設定(Macro)
#define OBJTIMING(p) TIMING(ProcTiming::##p)

std::string GetProcTimingName(ProcTiming proc);

//! スロット
template <class T>
struct SlotProc
{
    friend class Scene;
    friend class Object;
    friend class Component;

public:
    bool IsDirty() const { return dirty_; }

    void ResetDirty() { dirty_ = false; }

    const ProcTiming GetTiming() const { return timing_; }

    const Priority GetPriority() const { return priority_; }

    auto GetProc() { return proc_; }

    const auto GetName() const { return name_; }

    const bool IsUpdate() const { return is_update_; }

    const bool IsDraw() const { return is_draw_; }

    void SetProc(std::string name, ProcTiming timing, Priority prio, std::function<void(T)> func)
    {
        name_     = name;
        dirty_    = true;
        timing_   = timing;
        priority_ = prio;
        proc_     = func;

        is_update_ = false;
        is_draw_   = false;
        switch(timing) {
        default:
            break;

        case ProcTiming::PreUpdate:
        case ProcTiming::Update:
        case ProcTiming::LateUpdate:
        case ProcTiming::PostUpdate:
            is_update_ = true;
            break;

        case ProcTiming::PreDraw:
        case ProcTiming::Draw:
        case ProcTiming::LateDraw:
        case ProcTiming::PostDraw:
            is_draw_ = true;
            break;
        }
    }

private:
    std::string         name_{};
    ProcTiming          timing_    = ProcTiming::Draw;
    Priority            priority_  = Priority::NORMAL;
    bool                is_update_ = false;   //!< Updateにかかわるもの
    bool                is_draw_   = false;   //!< 表示しないでOFFになる
    sigslot::connection connect_{};
    bool                dirty_ = true;

    std::function<void(T)> proc_;

private:
    //--------------------------------------------------------------------
    //! @name Cereal処理
    //--------------------------------------------------------------------
    //@{
    CEREAL_SAVELOAD(arc, ver)
    {
        dirty_ = true;   //セーブするときにdirty_つける
        arc(name_, timing_, priority_, dirty_);
        // connect_ は再構築させる
    }

    //@}
};

//CEREAL_REGISTER_TYPE( SlotProc<void> )
//CEREAL_REGISTER_TYPE( SlotProc<float> )

template <class T>
using SlotProcs = std::unordered_map<std::string, SlotProc<T>>;
