//---------------------------------------------------------------------------
//! @file   ComponentFilterFade.h
//! @brief  フェードフィルターコンポーネント
//---------------------------------------------------------------------------
#pragma once

#include <System/Component/Component.h>

//===========================================================================
//! フェードフィルター
//===========================================================================
class ComponentFilterFade : public Component
{
public:
    //! コンストラクタ
    ComponentFilterFade(ObjectPtr owner);

    //  更新
    //! @param  [in]    delta   経過時間

    virtual void Update(f32 delta) override;

    // 描画
    virtual void Draw() override;

    // 終了
    virtual void Exit() override;

    // GUI描画
    virtual void GUI() override;

    void StartFadeIn(float time = 0.5f)
    {
        if(time == 0.0f)
            time += 0.0001f;

        fade_alpha_     = 0.0f;
        fade_add_alpha_ = 1.0f / time;
        finish_fade_    = false;
    }
    void StartFadeOut(float time = 0.5f)
    {
        if(time == 0.0f)
            time += 0.0001f;

        fade_alpha_     = 1.0f;
        fade_add_alpha_ = -1.0f / time;
        finish_fade_    = false;
    }

    bool IsFinishFade() { return finish_fade_; }

    bool IsFadeOut() { return fade_add_alpha_ < 0.0f; }
    bool IsFadeIn() { return fade_add_alpha_ > 0.0f; }

private:
    std::shared_ptr<Texture>  texture_work_;        //!< ワークテクスチャ
    std::shared_ptr<ShaderPs> shader_ps_;           //!< ピクセルシェーダー
    int                       parameter_cb_ = -1;   //!< [DxLib] 定数バッファ

    //----------------------------------------------------------
    //! @name   調整パラメーター
    //----------------------------------------------------------
    //@{

    f32 fade_alpha_   = 1.0f;   //!< フェード係数(0.0f～1.0f)
    int mosaic_width_ = 128;    //!< モザイクの最大ピクセル幅

    f32  fade_add_alpha_ = 1.0f;   //!< 1秒で変わる量
    bool finish_fade_    = true;
    //@}

private:
    //--------------------------------------------------------------------
    //! @name Cereal処理
    //--------------------------------------------------------------------
    //@{

    CEREAL_SAVELOAD(arc, ver) { arc(CEREAL_NVP(owner_), CEREAL_NVP(mosaic_width_), CEREAL_NVP(fade_alpha_)); }
    CEREAL_LOAD_AND_CONSTRUCT(ComponentFilterFade, arc, ver)
    {
        // オーナー情報を読み取り、componentを作成
        ObjectPtr owner;
        arc(cereal::make_nvp("owner_", owner));
        construct(owner);

        arc(cereal::make_nvp("mosaic_width_", construct->mosaic_width_));
        arc(cereal::make_nvp("fade_alpha_", construct->fade_alpha_));
    }

    //@}
};

CEREAL_REGISTER_TYPE(ComponentFilterFade)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Component, ComponentFilterFade)
