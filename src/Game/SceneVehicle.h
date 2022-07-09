//---------------------------------------------------------------------------
//! @file   SceneVehicle.h
//! @brief  自動車シーン
//---------------------------------------------------------------------------
#pragma once

#include <System/Scene.h>

class ComponentModel;
class ComponentCamera;

//===========================================================================
//! サンプルシーン
//===========================================================================
class SceneVehicle final : public Scene::Base
{
public:
    BP_CLASS_TYPE(SceneVehicle, Scene::Base)

    //! シーン名称
    std::string Name() override { return u8"自動車サンプル"; }

    bool Init() override;                //!< 初期化
    void Update(float delta) override;   //!< 更新
    void Draw() override;                //!< 描画
    void Exit() override;                //!< 終了
    void GUI() override;                 //!< GUI表示

private:
    std::shared_ptr<Model>                model_body_;   //!<  3Dモデル(ボディ)
    std::array<std::shared_ptr<Model>, 4> model_tire_;   //!<  3Dモデル(タイヤホイール)

    std::shared_ptr<ComponentCamera> camera_;   //!<  カメラコンポーネント

    //--------------------------------------------------------
    //!	@name 車の情報
    //--------------------------------------------------------
    //@{
    float3 position_ = float3(0.0f, 0.0f, 0.0f);   //!< 位置
    float3 velocity_ = float3(0.0f, 0.0f, 0.0f);   //!< 速度

    f32 steering_    = 0.5f;   //!< ハンドルの回転量(-1.0f～+1.0f)
    f32 gas_pedal_   = 0.5f;   //!< アクセルペダル(-1.0f～+1.0f)
    f32 brake_pedal_ = 0.5f;   //!< ブレーキペダル(-1.0f～+1.0f)

    matrix                mat_world_ = matrix::identity();   //!< 車の方向・位置情報の行列
    std::array<float3, 4> tire_world_position_;              //!<	タイヤのワールド座標
                                                             //@}
};
