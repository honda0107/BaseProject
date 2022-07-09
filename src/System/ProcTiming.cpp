//---------------------------------------------------------------------------
//! @file   ProcTiming.h
//! @brief  処理タイミング
//---------------------------------------------------------------------------

#include "ProcTiming.h"
#include <System/Object.h>

std::string GetProcTimingName(ProcTiming proc)
{
    if(ProcTiming::PreUpdate == proc)
        return "__system_PreUpdate";
    if(ProcTiming::Update == proc)
        return "__system_Update";
    if(ProcTiming::LateUpdate == proc)
        return "__system_LateUpdate";
    if(ProcTiming::PostUpdate == proc)
        return "__system_PostUpdate";
    if(ProcTiming::PreDraw == proc)
        return "__system_PreDraw";
    if(ProcTiming::Draw == proc)
        return "__system_Draw";
    if(ProcTiming::LateDraw == proc)
        return "__system_LateDraw";
    if(ProcTiming::PostDraw == proc)
        return "__system_PostDraw";

    if(ProcTiming::Shadow == proc)
        return "__system_Shadow";
    if(ProcTiming::Gbuffer == proc)
        return "__system_Gbuffer";
    if(ProcTiming::Light == proc)
        return "__system_Light";
    if(ProcTiming::HDR == proc)
        return "__system_HDR";

    if(ProcTiming::PrePhysics == proc)
        return "__system_PrePhysics";

    return "Unknown";
}
