//---------------------------------------------------------------------------
//! @file   Typedef.h
//! @brief  型定義
//---------------------------------------------------------------------------
#pragma once

#include <cstdint>

#include <vector>
#include <unordered_map>

//---------------------------------------------------------------------------
//! @name 数値型定義
//---------------------------------------------------------------------------
//@{

using s8  = int8_t;     //!<  8bit 符号あり整数
using s16 = int16_t;    //!< 16bit 符号あり整数
using s32 = int32_t;    //!< 32bit 符号あり整数
using s64 = int64_t;    //!< 64bit 符号あり整数
using u8  = uint8_t;    //!<  8bit 符号なし整数
using u16 = uint16_t;   //!< 16bit 符号なし整数
using u32 = uint32_t;   //!< 32bit 符号なし整数
using u64 = uint64_t;   //!< 64bit 符号なし整数
using f32 = float;      //!< 単精度浮動小数点数
using f64 = double;     //!< 倍精度浮動小数点数

//@}
//---------------------------------------------------------------------------
//! @name ポインタ定義
//---------------------------------------------------------------------------
//@{

#define USING_PTR(obj)                                                                                                 \
    class obj;                                                                                                         \
    using obj##Ptr          = std::shared_ptr<obj>;                                                                    \
    using obj##WeakPtr      = std::weak_ptr<obj>;                                                                      \
    using obj##UniquePtr    = std::unique_ptr<obj>;                                                                    \
    using obj##PtrVec       = std::vector<obj##Ptr>;                                                                   \
    using obj##WeakPtrVec   = std::vector<obj##WeakPtr>;                                                               \
    using obj##UniquePtrVec = std::vector<obj##UniquePtr>;                                                             \
    using obj##Vec          = std::vector<obj*>;                                                                       \
    using obj##PtrMap       = std::unordered_map<std::string, obj##Ptr>

//@}
