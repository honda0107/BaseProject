#pragma once

#include <System/Graphics/Frustum.h>
#include <System/Component/Component.h>
#include <System/Cereal.h>

USING_PTR(ComponentCamera);

class ComponentCamera : public Component
{
    friend class Object;

public:
    ComponentCamera(ObjectPtr owner)
        : Component(owner)
    {
    }

    virtual void Init() override;                //!< カメラ初期化
    virtual void PreUpdate() override;           //!< カメラ更新
    virtual void Update(float delta) override;   //!< カメラ更新
    virtual void PostUpdate() override;          //!< カメラ更新
    virtual void Draw() override;   //!< 標準カメラや別のカメラから見た時のカメラモデルを表示する
    virtual void Exit() override;   //!< カメラ終了処理
    virtual void GUI() override;    //!< カメラGUI処理

    //! @brief カメラの位置設定
    void SetCameraTransform();

    //! @brief カレントカメラに設定します
    ComponentCameraPtr SetCurrentCamera()
    {
        camera_status_.on(CameraBit::ChangeReq);
        return std::dynamic_pointer_cast<ComponentCamera>(shared_from_this());
    }

    static ComponentCameraWeakPtr GetCurrentCamera() { return current_camera_; }

    //! @brief 画角設定
    //! @param fov          画角(角度0~360[通常45~90くらい])
    //! @param aspect_ratio アスペクト比
    std::shared_ptr<ComponentCamera> SetPerspective(float fov, float aspect_ratio = 16.0f / 9.0f)
    {
        fovy_         = fov;
        aspect_ratio_ = aspect_ratio;
        return std::dynamic_pointer_cast<ComponentCamera>(shared_from_this());
    }

    //! @brief カメラポジションとターゲットの設定
    //! @param position カメラポジション
    //! @param look_at  注視点ターゲット
    //! @param up       上ベクトル
    std::shared_ptr<ComponentCamera>
    SetPositionAndTarget(float3 position, float3 look_at, float3 up = {0.0f, 1.0f, 0.0f})
    {
        position_ = position;
        look_at_  = look_at;
        up_       = up;

        return std::dynamic_pointer_cast<ComponentCamera>(shared_from_this());
    }

    //! @brief Near/Farの設定
    //! @param near_z   近クリップ面のZ値
    //! @param far_z    遠クリップ面のZ値
    std::shared_ptr<ComponentCamera> SetNearFar(float near_z, float far_z)
    {
        near_z_ = near_z;
        far_z_  = far_z;

        return std::dynamic_pointer_cast<ComponentCamera>(shared_from_this());
    }

    //! @brief カメラの更新タイミングを設定する
    //! @param timing 処理するタイミング
    //! @detail ProcTiming::PreUpdate カメラを設定してそれに対してオブジェクトを動かす時に利用する
    //! @detail ProcTiming::Update プレイヤーをカメラを設定して追いかける時などに利用する(Default)
    void SetUpdateTiming(ProcTiming timing)
    {
        if(timing == ProcTiming::Update) {
            camera_status_.on(CameraBit::UpdateOnUpdate);
            camera_status_.off(CameraBit::UpdateOnPreUpdate);
            camera_status_.off(CameraBit::UpdateOnPostUpdate);
        }
        else if(timing == ProcTiming::PreUpdate) {
            camera_status_.on(CameraBit::UpdateOnPreUpdate);
            camera_status_.off(CameraBit::UpdateOnUpdate);
            camera_status_.off(CameraBit::UpdateOnPostUpdate);
        }
        else if(timing == ProcTiming::PostUpdate) {
            camera_status_.on(CameraBit::UpdateOnPostUpdate);
            camera_status_.off(CameraBit::UpdateOnPreUpdate);
            camera_status_.off(CameraBit::UpdateOnUpdate);
        }
    }

    //! @brief カメラ位置の取得
    //! @return カメラ位置
    float3 GetPosition() const;

    //! @brief カメラターゲットの取得
    //! @return カメラターゲット
    float3 GetTarget() const;

    struct CameraRay
    {
        float3 start;
        float3 end;
    };

    //! @brief マウス位置からカメラレイを取得します
    //! @param mouse_pos_x マウスX位置
    //! @param mouse_pos_y マウスY位置
    //! @return カメラレイ
    CameraRay MousePositionRay(int mouse_pos_x, int mouse_pos_y);

    //---------------------------------------------------------------------------
    //! モデルステータス
    //---------------------------------------------------------------------------
    enum struct CameraBit : u32
    {
        Initialized,          //!< 初期化済み
        EnableTimeLine,       //!< TimelineComponentを利用して移動する
        ChangeReq,            //!< カレントカメラ変更リクエスト
        Current,              //!< 現状のカレントのカメラ
        UpdateOnPreUpdate,    //!< PreUpdate時に更新する
        UpdateOnUpdate,       //!< Update時に更新する
        UpdateOnPostUpdate,   //!< PostUpdate時に更新する
        ShowFrustum,          //!< Frustumを描画する
        DebugCameara,         //!< 自分がデバッグカメラ
    };

    void SetCameraStatus(CameraBit bit, bool on) { camera_status_.set(bit, on); }
    bool GetCameraStatus(CameraBit bit) { return camera_status_.is(bit); }

private:
    Status<CameraBit> camera_status_;   //!< 状態

    float3 position_ = {0, 5, -10};   //!< カメラ座標
    float3 look_at_  = {0, 0, 0};     //!< 注視点
    float3 up_       = {0, 1, 0};     //!< 上ベクトル

    float aspect_ratio_ = 16.0f / 9.0f;   //!< アスペクト比
    float fovy_         = 45.0f;          //!< FieldOfView
    float near_z_       = 0.01f;          //!< Near
    float far_z_        = 10000.0f;       //!< Far

    matrix mat_view_;   //!< ビュー行列。Updateで計算、Draw手前で使用している
    matrix mat_proj_;   //!< 投影行列。Updateで計算、Draw手前で使用している

    Frustum frustum_{};

    static ComponentCameraWeakPtr current_camera_;

    //----------------------------------------------------------
    //! @name   定数バッファ
    //----------------------------------------------------------
    //@{

    struct CameraInfo
    {
        matrix mat_view_;       //!< ビュー行列
        matrix mat_proj_;       //!< 投影行列
        float3 eye_position_;   //!< カメラの位置
    };

    int cb_camera_info_ = -1;   //!< [DxLib] カメラ情報定数バッファハンドル

    //@}

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
        arc(cereal::make_nvp("owner", owner_));                         //< オーナー
        arc(cereal::make_nvp("camera_status", camera_status_.get()));   //< カメラステート
        arc(CEREAL_NVP(position_), CEREAL_NVP(look_at_),
            CEREAL_NVP(up_));   //< カメラ位置とターゲット
        arc(cereal::make_nvp("aspect_ratio", aspect_ratio_), cereal::make_nvp("fovy", fovy_));   //< アスペクト比と画角
        arc(cereal::make_nvp("near_z", near_z_), cereal::make_nvp("far_z", far_z_));             //< Near/Far
        arc(CEREAL_NVP(current_camera_));
    }

    //! @brief ロード
    // @param arc アーカイバ
    // @param ver バージョン
    CEREAL_LOAD_AND_CONSTRUCT(ComponentCamera, arc, ver)
    {
        ObjectPtr owner;
        arc(CEREAL_NVP(owner));
        construct(owner);
        //! @todo リカバリー camera_status
        arc(cereal::make_nvp("camera_status", construct->camera_status_.get()));
        arc(cereal::make_nvp("current_camera_", construct->current_camera_));

        float3 position, look_at, up;
        arc(cereal::make_nvp("position_", position));
        arc(cereal::make_nvp("look_at_", look_at));
        arc(cereal::make_nvp("up_", up));
        construct->SetPositionAndTarget(position, look_at, up);

        float aspect_ratio, fovy;
        arc(CEREAL_NVP(aspect_ratio), CEREAL_NVP(fovy));   //< アスペクト比と画角

        float near_z, far_z;
        arc(CEREAL_NVP(near_z), CEREAL_NVP(far_z));   //< Near/Far

        construct->SetPerspective(fovy, aspect_ratio);
        construct->SetNearFar(near_z, far_z);
    }

    //@}
};

CEREAL_REGISTER_TYPE(ComponentCamera)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Component, ComponentCamera)
