//---------------------------------------------------------------------------
//! @file   ComponentTransform.h
//! @brief  トランスフォームコンポーネント(座標と姿勢と大きさ)
//---------------------------------------------------------------------------
#pragma once
#include <System/Component/Component.h>
#include <System/Cereal.h>
#include <System/VectorMath.h>
#include <ImGuizmo/ImGuizmo.h>
#include <DxLib.h>
#include <memory>

// ImGuizmoのMatrixからEulerに変換する際に全軸に180度変換がかかってしまうため修正
extern void DecomposeMatrixToComponents(const float* matx, float* translation, float* rotation, float* scale);

template <class T>
class IMatrix
{
public:
    //! @brief TransformのMatrix情報を取得します
    //! @return Transform の Matrix
    virtual matrix& Matrix() = 0;

#if 0
	//! @brief TransformのMatrix情報を取得します
	//! @return Transform の Matrix
	[[deprecated( "Matrix()を使用してください" )]] virtual matrix& GetMatrix()
	{
		return Matrix();
	};
#endif

    virtual std::shared_ptr<T> SharedThis() = 0;

    //! @brief ワールドMatrixの取得
    //! @return 自分にかかわる他のコンポーネントも含めた位置
    virtual const matrix GetWorldMatrix() = 0;

    //! @brief 1フレーム前のワールドMatrixの取得
    //! @return 他のコンポーネントも含めた位置
    virtual const matrix GetOldWorldMatrix() = 0;

    //! @brief TransformのMatrix情報を取得します
    //! @return Transform の Matrix
    const matrix GetMatrix()
    {
        return Matrix();
    }

    auto SetMatrix(const matrix& mat)
    {
        Matrix() = mat;
        return SharedThis();
    }

    //! @brief TransformのMatrix情報を取得します
    //! @return Transform の Matrix
    float* GetMatrixFloat()
    {
        return Matrix().f32_128_0;
    }
#if 0
	//! @brief TransformのMatrix情報を取得します
	//! @return Transform の Matrix
	const float* GetMatrixFloat() const
	{
		return GetMatrix().f32_128_0;
	}
#endif
    //! @brief 位置をセットします
    //! @param translate 位置
    //! @return 自分のSharedPtr
    auto SetTranslate(float3 translate)
    {
        (float4&)Matrix().translateVector() = {translate, 1};

        return SharedThis();
    }

    //! @brief 現在の位置から移動させます
    //! @param translate 移動させる量
    //! @return 自分のSharedPtr
    auto AddTranslate(float3 translate)
    {
        SetTranslate(GetTranslate() + translate);

        return SharedThis();
    }

    //! @brief 現在の位置を取得
    //! @return Transformの位置
    const float3 GetTranslate()
    {
        return GetMatrix().translate();
    }

    //! @brief 現在の位置を取得
    //! @return Transformの位置
    auto& Translate()
    {
        return (float3&)Matrix().translateVector();
    }

    [[deprecated("GetTranslate()を使用してください。代入にはTranslate()を使用ください")]] float4 GetPosition()
    {
        return (float4)Matrix().translateVector();
    }

    //! @brief X軸を取得する
    //! @return TransformのX軸
    float3 GetVectorAxisX()
    {
        return GetMatrix().axisX();
    }
    [[deprecated(
        "古い形式です。GetVectorAxisX()を変更してください。エラーが出る場合はVectorAxisX()に変更します")]] float3
    GetXVector()
    {
        return GetVectorAxisX();
    }

    //! @brief Y軸を取得する
    //! @return TransformのY軸
    float3 GetVectorAxisY()
    {
        return GetMatrix().axisY();
    }
    [[deprecated(
        "古い形式です。GetVectorAxisY()を変更してください。エラーが出る場合はVectorAxisY()に変更します")]] float3
    GetYVector()
    {
        return GetVectorAxisY();
    }

    //! @brief Z軸を取得する
    //! @return TransformのZ軸
    float3 GetVectorAxisZ()
    {
        return GetMatrix().axisX();
    }
    [[deprecated(
        "古い形式です。GetVectorAxisZ()を変更してください。エラーが出る場合はVectorAxisZ()に変更します")]] float3
    GetZVector()
    {
        return GetVectorAxisZ();
    }

    //! @brief X軸を取得する
    //! @return TransformのX軸
    auto& VectorAxisX()
    {
        return (float3&)Matrix().axisVectorX();
    }

    //! @brief Y軸を取得する
    //! @return TransformのY軸
    auto& VectorAxisY()
    {
        return (float3&)Matrix().axisVectorY();
    }

    //! @brief Z軸を取得する
    //! @return TransformのZ軸
    auto& VectorAxisZ()
    {
        return (float3&)Matrix().axisVectorZ();
    }

    //! @brief 位置を取得する
    //! @return Transformの位置
    float* TranslateFloat()
    {
        return (float*)Matrix().f32_128_3;
    }

    [[deprecated("TranslateFloat()を使用してください")]] float* GetTranslateFloat()
    {
        return TranslateFloat();
    }

    [[deprecated("TranslateFloat()を使用してください")]] float* GetPositionFloat()
    {
        return GetTranslateFloat();
    }

    //! @brief X軸を取得する
    //! @return ComponentTransformのX軸
    float* VectorAxisXFloat()
    {
        return (float*)Matrix().f32_128_0;
    }

    [[deprecated("古い形式です。VectorAxisXFloat()に変更してください")]] float* GetXVectorFloat()
    {
        return VectorAxisXFloat();
    }

    //! @brief Y軸を取得する
    //! @return ComponentTransformのY軸
    float* VectorAxisYFloat()
    {
        return (float*)Matrix().f32_128_1;
    }

    [[deprecated("古い形式です。VectorAxisYFloat()に変更してください")]] float* GetYVectorFloat()
    {
        return VectorAxisYFloat();
    }

    //! @brief Z軸を取得する
    //! @return ComponentTransformのZ軸
    float* VectorAxisZFloat()
    {
        return (float*)Matrix().f32_128_2;
    }

    [[deprecated("古い形式です。VectorAxisZFloat()に変更してください")]] float* GetZVectorFloat()
    {
        return VectorAxisZFloat();
    }

    //! @brief X軸を設定する
    //! @param vec ベクトル
    //! @return 自分のSharedPtr
    auto SetVectorAxisX(float3 vec)
    {
        Matrix()._11_12_13 = vec;
        return SharedThis();
    }

    [[deprecated("古い形式です。SetVectorAxisX()に変更してください")]] auto SetXVector(float3 vec)
    {
        return SetVectorAxisX(vec);
    }

    //! @brief Y軸を設定する
    //! @param vec ベクトル
    //! @return 自分のSharedPtr
    auto SetVectorAxisY(float3 vec)
    {
        Matrix()._21_22_23 = vec;
        return SharedThis();
    }

    [[deprecated("古い形式です。SetVectorAxisY()に変更してください")]] auto SetYVector(float3 vec)
    {
        return SetVectorAxisY(vec);
    }

    //! @brief Z軸を設定する
    //! @param vec ベクトル
    //! @return 自分のSharedPtr
    auto SetVectorAxisZ(float3 vec)
    {
        Matrix()._31_32_33 = vec;
        return SharedThis();
    }

    [[deprecated("古い形式です。SetVectorAxisZ()に変更してください")]] auto SetZVector(float3 vec)
    {
        return SetVectorAxisZ(vec);
    }

    //! @brief 軸回転量を設定する
    //! @param xyz 軸に合わせた回転量
    //! @return 自分のSharedPtr
    auto SetRotationAxisXYZ(float3 xyz)
    {
        float matrixTranslation[3], matrixRotation[3], matrixScale[3];
        DecomposeMatrixToComponents(GetMatrixFloat(), matrixTranslation, matrixRotation, matrixScale);

        matrixRotation[0] = xyz.x;
        matrixRotation[1] = xyz.y;
        matrixRotation[2] = xyz.z;

        ImGuizmo::RecomposeMatrixFromComponents(matrixTranslation, matrixRotation, matrixScale, GetMatrixFloat());
        return SharedThis();
    }

    [[deprecated("古い命名です。SetRotationAxisXYZ()に変更してください")]] auto SetRotationXYZAxis(float3 xyz)
    {
        return SetRotationAxisXYZ(xyz);
    }

    //! @brief 軸回転量を現在の状態から追加する
    //! @param xyz 軸に合わせた回転量
    //! @return 自分のSharedPtr
    auto AddRotationAxisXYZ(float3 xyz)
    {
        float matrixTranslation[3], matrixRotation[3], matrixScale[3];
        DecomposeMatrixToComponents(GetMatrixFloat(), matrixTranslation, matrixRotation, matrixScale);

        matrixRotation[0] += xyz.x;
        matrixRotation[1] += xyz.y;
        matrixRotation[2] += xyz.z;

        ImGuizmo::RecomposeMatrixFromComponents(matrixTranslation, matrixRotation, matrixScale, GetMatrixFloat());
        return SharedThis();
    }

    [[deprecated("古い命名です。AddRotationAxisXYZ()に変更してください")]] auto AddRotationXYZAxis(float3 xyz)
    {
        return AddRotationAxisXYZ(xyz);
    }

    //! @brief 軸回転量を取得する
    //! @details 変数としては存在しないため代入は不可
    //! @return xyzローテート量
    const float3 GetRotationAxisXYZ()
    {
        float matrixTranslation[3], matrixRotation[3], matrixScale[3];
        DecomposeMatrixToComponents(GetMatrixFloat(), matrixTranslation, matrixRotation, matrixScale);

        return *(float3*)matrixRotation;
    }

    [[deprecated("古い命名です。GetRotationAxisXYZ()に変更してください")]] const float GetRotationXYZAxis()
    {
        return GetRotationAxisXYZ();
    }

    //! @brief  スケールの設定
    //! @param xyz スケール
    //! @return 自分のSharedPtr
    auto SetScaleAxisXYZ(float3 scale)
    {
        float matrixTranslation[3], matrixRotation[3], matrixScale[3];
        DecomposeMatrixToComponents(GetMatrixFloat(), matrixTranslation, matrixRotation, matrixScale);

        matrixScale[0] = scale.x;
        matrixScale[1] = scale.y;
        matrixScale[2] = scale.z;

        ImGuizmo::RecomposeMatrixFromComponents(matrixTranslation, matrixRotation, matrixScale, GetMatrixFloat());
        return SharedThis();
    }

    [[deprecated("古い命名です。SetScaleAxisXYZ()に変更してください")]] auto SetScaleXYZAxis(float3 vec)
    {
        return SetScaleAxisXYZ(vec);
    }

    //! @brief  現在のスケールから掛け合わせ倍率スケール
    //! @param xyz スケール
    //! @return 自分のSharedPtr
    auto MulScaleAxisXYZ(float3 scale)
    {
        float matrixTranslation[3], matrixRotation[3], matrixScale[3];
        DecomposeMatrixToComponents(GetMatrixFloat(), matrixTranslation, matrixRotation, matrixScale);

        matrixScale[0] *= scale.x;
        matrixScale[1] *= scale.y;
        matrixScale[2] *= scale.z;

        ImGuizmo::RecomposeMatrixFromComponents(matrixTranslation, matrixRotation, matrixScale, GetMatrixFloat());
        return SharedThis();
    }

    [[deprecated("古い命名です。SetScaleAxisXYZ()に変更してください")]] auto MulScaleXYZAxis(float3 vec)
    {
        return MulScaleAxisXYZ(vec);
    }

    //! @brief スケールの取得
    //! @details 変数としては存在しないため代入は不可
    //! @return スケール値
    const float3 GetScaleAxisXYZ()
    {
        float matrixTranslation[3], matrixRotation[3], matrixScale[3];
        DecomposeMatrixToComponents(GetMatrixFloat(), matrixTranslation, matrixRotation, matrixScale);

        return *(float3*)matrixScale;
    }

    [[deprecated("古い命名です。GetScaleAxisXYZ()に変更してください")]] const float3 GetScaleXYZAxis(float3 vec)
    {
        return GetScaleAxisXYZ(vec);
    }
};

USING_PTR(ComponentTransform);

//! トランスフォーム
class ComponentTransform
    : public Component
    , public IMatrix<ComponentTransform>
{
    friend class Object;

public:
    ComponentTransform(ObjectPtr owner)
        : Component(owner)
    {
        Matrix() = matrix::identity();
    }

    virtual void PostUpdate() override;
    virtual void GUI() override;   //!< GUI処理

    //---------------------------------------------------------------------------
    //! @name IMatrixインターフェースの利用するための定義
    //---------------------------------------------------------------------------
    //@{

    matrix& Matrix() override { return transform_; }   //!< マトリクス取得

    ComponentTransformPtr SharedThis() override
    {
        return std::dynamic_pointer_cast<ComponentTransform>(shared_from_this());
    }

    //! @brief ワールドMatrixの取得
    //! @return 他のコンポーネントも含めた位置
    virtual const matrix GetWorldMatrix() override { return GetMatrix(); }

    //! @brief 1フレーム前のワールドMatrixの取得
    //! @return 他のコンポーネントも含めた位置
    virtual const matrix GetOldWorldMatrix() override { return old_transform_; }
    //@}

private:
    matrix transform_;
    matrix old_transform_;   //!< 1フレーム前の位置

    bool                is_guizmo_       = false;                 //!< ギズモ使用
    ImGuizmo::OPERATION gizmo_operation_ = ImGuizmo::TRANSLATE;   //!< Gizmo処理選択
    ImGuizmo::MODE      gizmo_mode_      = ImGuizmo::LOCAL;       //!< Gizmo Local/Global設定
    static Component*   select_component_;                        //!< どのTransformが処理されているか

private:
    //--------------------------------------------------------------------
    //! @name Cereal処理
    //--------------------------------------------------------------------
    //@{
    CEREAL_SAVELOAD(arc, ver)
    {
        arc(cereal::make_nvp("owner", owner_));
        arc(cereal::make_nvp("transform", transform_));
        arc(cereal::make_nvp("old_transform", transform_));
    }

    CEREAL_LOAD_AND_CONSTRUCT(ComponentTransform, arc, ver)
    {
        // オーナー情報を読み取りその後マトリクスを初期化する
        ObjectPtr owner;
        arc(CEREAL_NVP(owner));
        construct(owner);

        arc(cereal::make_nvp("transform", construct->transform_));
    }
    //@}
};

CEREAL_REGISTER_TYPE(ComponentTransform)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Component, ComponentTransform)
