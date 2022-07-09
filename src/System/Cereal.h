//---------------------------------------------------------------------------
//! @file   Cereal.h
//! @brief  シリアル組み込み用
//---------------------------------------------------------------------------
#pragma once

#include <cereal/cereal.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/types/array.hpp>
#include <cereal/types/unordered_map.hpp>

#include <System/VectorMath.h>
//---------------------------------------------------------------------------
//! 非侵入型定義
//---------------------------------------------------------------------------

namespace cereal
{
template <class Archive>
void serialize(Archive& arc, hlslpp::float2& f)
{
    arc(cereal::make_nvp("x", (float&)f.f32[0]), cereal::make_nvp("y", (float&)f.f32[1]));
}

template <class Archive>
void serialize(Archive& arc, hlslpp::float3& f)
{
    arc(cereal::make_nvp("x", (float&)f.f32[0]),
        cereal::make_nvp("y", (float&)f.f32[1]),
        cereal::make_nvp("z", (float&)f.f32[2]));
}

template <class Archive>
void serialize(Archive& arc, hlslpp::float4& f)
{
    arc(cereal::make_nvp("x", (float&)f.f32[0]),
        cereal::make_nvp("y", (float&)f.f32[1]),
        cereal::make_nvp("z", (float&)f.f32[2]),
        cereal::make_nvp("w", (float&)f.f32[3]));
}

template <class Archive>
void save(Archive& arc, const matrix& m)
{
    arc((float4&)m.f32_128_0, (float4&)m.f32_128_1, (float4&)m.f32_128_2, (float4&)m.f32_128_3);
}

template <class Archive>
void load(Archive& arc, matrix& m)
{
    arc((float4&)m.f32_128_0, (float4&)m.f32_128_1, (float4&)m.f32_128_2, (float4&)m.f32_128_3);
}

}   // namespace cereal

//---------------------------------------------------------------------------
//! Cereal
//---------------------------------------------------------------------------
//! ロード・セーブ両方同じ処理の時に使用します
#define CEREAL_SAVELOAD(arc, ver)                                                                                      \
    friend class cereal::access;                                                                                       \
    template <class Archive>                                                                                           \
    void serialize(Archive& arc, [[maybe_unused]] std::uint32_t const ver)

//! セーブ処理のみの時に使用します (※現在処理されないと思われます)
#define CEREAL_SAVE(arc, ver)                                                                                          \
    friend class cereal::access;                                                                                       \
    template <class Archive>                                                                                           \
    void save(Archive& arc, [[maybe_unused]] std::uint32_t const ver) const

//! ロード処理のみの時に使用します
#define CEREAL_LOAD(arc, ver)                                                                                          \
    friend class cereal::access;                                                                                       \
    template <class Archive>                                                                                           \
    void load(Archive& arc, [[maybe_unused]] std::uint32_t const ver)

//! ロード処理の時、デフォルトコンストラクタ以外を使用したいときに使用します
#define CEREAL_LOAD_AND_CONSTRUCT(cls, arc, ver)                                                                       \
    template <class Archive>                                                                                           \
    static void load_and_construct(Archive&                             arc,                                           \
                                   cereal::construct<cls>&              construct,                                     \
                                   [[maybe_unused]] std::uint32_t const ver)

//! ロード・セーブ両方同じ処理の時に使用します (CPPにて組み込む場合)
#define CEREAL_SAVELOAD_CPP(cls, arc, ver)                                                                             \
    template <class Archive>                                                                                           \
    void cls::serialize(Archive& arc, [[maybe_unused]] std::uint32_t const ver)

//! セーブ処理のみの時に使用します (CPPにて組み込む場合) (※現在処理されないと思われます)
#define CEREAL_SAVE_CPP(cls, arc, ver)                                                                                 \
    template <class Archive>                                                                                           \
    void cls::save(Archive& arc, [[maybe_unused]] std::uint32_t const ver) const

//! ロード処理のみの時に使用します (CPPにて組み込む場合)
#define CEREAL_LOAD_CPP(cls, arc, ver)                                                                                 \
    friend class cereal::access;                                                                                       \
    template <class Archive>                                                                                           \
    void cls::load(Archive& arc, [[maybe_unused]] std::uint32_t const ver)

//! ロード処理の時、デフォルトコンストラクタ以外を使用したいときに使用します (CPPにて組み込む場合)
//! @attention 変数名はconstruct以外使うと再定義になってしまうため変数化していない。
#define CEREAL_LOAD_AND_CONSTRUCT_CPP(cls, arc, ver)                                                                   \
    template <class Archive>                                                                                           \
    void cls::load_and_construct(Archive&                             arc,                                             \
                                 cereal::construct<cls>&              construct,                                       \
                                 [[maybe_unused]] std::uint32_t const ver)

//! ロード処理の時、デフォルトコンストラクタ以外を使用したいときに使用します(外部)
//! @attention 変数名はconstruct以外使うと再定義になってしまうため変数化していない。
#define CEREAL_LOAD_AND_CONSTRUCT_BEGIN(cls, arc, ver)                                                                 \
    namespace cereal                                                                                                   \
    {                                                                                                                  \
    template <>                                                                                                        \
    struct LoadAndConstruct<cls>                                                                                       \
    {                                                                                                                  \
        template <class Archive>                                                                                       \
        static void load_and_construct(Archive&                             arc,                                       \
                                       cereal::construct<cls>&              construct,                                 \
                                       [[maybe_unused]] std::uint32_t const ver)

//! ロード処理の時、デフォルトコンストラクタ以外を使用したいときに使用します(外部)
//! @brief デフォルトコンストラクタ以外の初期化の場合に利用する(END)
#define CEREAL_LOAD_AND_CONSTRUCT_END()                                                                                \
    }                                                                                                                  \
    ;                                                                                                                  \
    }   // namespace cereal
