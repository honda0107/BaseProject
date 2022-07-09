//---------------------------------------------------------------------------
//! @file   DebugCamera.h
//! @brief  デバッグカメラ
//---------------------------------------------------------------------------
#pragma once

#include <System/Component/ComponentCamera.h>

namespace DebugCamera
{
enum Control : int
{
    Unity,
    UnrealEngine,
    Maya,
};

//! @brief デバックカメラを使用する
//! @param use 使用するかどうか
void Use(bool use);

//! @brief デバッグカメラを使用中かどうか
//! @retval true : デバッグカメラ使用中
bool IsUse();

//! @brief デバッグカメラの移動方法設定
//! @param mode 移動方式
void SetControl(Control mode);

ComponentCameraWeakPtr GetCamera();

}   // namespace DebugCamera
