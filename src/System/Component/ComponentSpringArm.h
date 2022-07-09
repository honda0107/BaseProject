#pragma once

#include <System/Component/Component.h>
#include <System/Cereal.h>

USING_PTR(ComponentSpringArm);

class ComponentSpringArm : public Component
{
    friend class Object;

public:
    ComponentSpringArm(ObjectPtr owner)
        : Component(owner)
    {
    }

    virtual void Init() override;                //!< 初期化
    virtual void Update(float delta) override;   //!< 更新
    virtual void PostUpdate() override;          //!< 更新
    virtual void Draw() override;                //!< デバッグ描画
    virtual void Exit() override;                //!< 終了処理
    virtual void GUI() override;                 //!< GUI処理

    //! @brief スプリングアームの先の位置の取得
    //! @return 位置の取得
    matrix GetPutOnMatrix() const;

    //---------------------------------------------------------------------------
    //! モデルステータス
    //---------------------------------------------------------------------------
    enum struct SpringArmBit : u32
    {
        Initialized,      //!< 初期化済み
        EnableTimeLine,   //!< TimelineComponentを利用して移動する
    };

    void SetCameraStatus(SpringArmBit bit, bool on) { spring_arm_status_.set(bit, on); }
    bool GetCameraStatus(SpringArmBit bit) { return spring_arm_status_.is(bit); }

    void SetSpringArmObject(ObjectPtr object) { spring_arm_object_ = object; }

    void SetSpringArmVector(float3 vec) { spring_arm_vector_ = vec; }

    void SetSpringArmOffset(float3 offset) { spring_arm_offset_ = offset; }

    ObjectWeakPtr GetSpringArmObject() { return spring_arm_object_; }

    void SetSpringArmStrong(float strong) { spring_arm_strong_ = strong; }
    void SetSpringArmReturn(float ret) { spring_arm_strong_ = ret; }

private:
    Status<SpringArmBit> spring_arm_status_;   //!< 状態

    float3 spring_arm_vector_{0, 3, 10};
    float3 spring_arm_offset_{0, 3, 0};

    float spring_arm_strong_ = 1.00f;
    float spring_arm_return_ = 0.01f;

    float spring_arm_length_     = 0.0f;
    float spring_arm_length_now_ = 0.0f;
    float spring_arm_vecspd_     = 0.0f;

    ObjectWeakPtr spring_arm_object_;

private:
    //--------------------------------------------------------------------
    //! @name Cereal処理
    //--------------------------------------------------------------------
    //@{

    //! @brief セーブ
    // @param arc アーカイバ
    // @param ver バージョン
    CEREAL_SAVELOAD(arc, ver)
    {
        arc(cereal::make_nvp("owner", owner_));                                 //< オーナー
        arc(cereal::make_nvp("spring_arm_status", spring_arm_status_.get()));   //< カメラステート
        arc(CEREAL_NVP(spring_arm_offset_));
    }

    //! @brief ロード
    // @param arc アーカイバ
    // @param ver バージョン
    CEREAL_LOAD_AND_CONSTRUCT(ComponentSpringArm, arc, ver)
    {
        ObjectPtr owner;
        arc(CEREAL_NVP(owner));
        construct(owner);
        //! @todo リカバリー camera_status
        arc(cereal::make_nvp("spring_arm_status", construct->spring_arm_status_.get()));
        arc(cereal::make_nvp("spring_arm_offset_", construct->spring_arm_offset_));
    }

    //@}
};

CEREAL_REGISTER_TYPE(ComponentSpringArm)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Component, ComponentSpringArm)
