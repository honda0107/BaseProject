#include <System/Component/ComponentSpringArm.h>
#include <System/Component/ComponentTransform.h>
#include <System/Object.h>
#include <System/Scene.h>
#include <System/ImGui.h>

//---------------------------------------------------------
//! 初期化
//---------------------------------------------------------
void ComponentSpringArm::Init()
{
    __super::Init();

    /*
	SetProc(
		"SpringArm",
		[]() 
		{

		},
		ProcTiming::Update,
		Priority::LOWEST );
	*/
    // PreUpdateの最後で自分の位置を設定する(ターゲットの移動の後)
    Scene::GetCurrentScene()->SetPriority(shared_from_this(), ProcTiming::PreUpdate, Priority::LOWEST);
    // 向きはPostUpdateの最初で行っておく(カメラのPost向きよりも前になるように)
    Scene::GetCurrentScene()->SetPriority(shared_from_this(), ProcTiming::PostUpdate, Priority::HIGHEST);

    spring_arm_status_.on(SpringArmBit::Initialized);
}

//---------------------------------------------------------
//! 更新
//---------------------------------------------------------
void ComponentSpringArm::Update([[maybe_unused]] float delta)
{
#if 0
	float3 old_pos = GetOwner()->GetTranslate();

	auto mat = GetPutOnMatrix();
	float3 new_pos = mat.translate();

	if( auto object = spring_arm_object_.lock() )
	{
		spring_arm_length_	   = length( spring_arm_vector_ );
		spring_arm_length_now_ = length( old_pos - object->GetTranslate() );

		auto len = spring_arm_length_ * spring_arm_strong_ + spring_arm_length_now_ * ( 1.0f - spring_arm_strong_ ) + spring_arm_vecspd_;
		spring_arm_vecspd_ = ( spring_arm_length_ - spring_arm_length_now_ ) * spring_arm_return_;

		mat._41_42_43 = ( new_pos - object->GetTranslate() ) * ( len / spring_arm_length_ ) + object->GetTranslate();
	}
	GetOwner()->SetMatrix( mat );
#endif
    GetOwner()->SetMatrix(GetPutOnMatrix());
}

void ComponentSpringArm::PostUpdate()
{
    // 自分の位置はそのまま使用し、ターゲット位置のほうを向くようにする
    if(auto object = spring_arm_object_.lock()) {
        auto   my_pos     = GetOwner()->GetTranslate();
        auto   target_pos = object->GetTranslate();
        matrix mat        = HelperLib::Math::LookAtMatrixForObject(my_pos, target_pos);

        float3 ofs = mul(float4(spring_arm_offset_, 0), mat).xyz;
        mat        = HelperLib::Math::LookAtMatrixForObject(my_pos, target_pos + ofs);

        GetOwner()->SetMatrix(mat);
    }
}

//! @brief スプリングアームの先のマトリクス取得
//! @return マトリクス
matrix ComponentSpringArm::GetPutOnMatrix() const
{
    float3 vec = spring_arm_vector_;
    matrix mat = matrix::identity();

    if(auto object = spring_arm_object_.lock()) {
        if(auto transform = object->GetComponent<ComponentTransform>()) {
            float3 target = transform->GetTranslate();
            auto   pos    = mul(float4(vec, 1), transform->GetMatrix()).xyz;

            mat = HelperLib::Math::LookAtMatrixForObject(pos, target);
        }
    }

    return mat;
}

//---------------------------------------------------------
//! デバッグ表示
//---------------------------------------------------------
void ComponentSpringArm::Draw()
{
}

//---------------------------------------------------------
//! カメラ終了処理
//---------------------------------------------------------
void ComponentSpringArm::Exit()
{
    __super::Exit();
}

//---------------------------------------------------------
//! GUI処理
//---------------------------------------------------------
void ComponentSpringArm::GUI()
{
    assert(GetOwner());
    auto obj_name = GetOwner()->GetName();

    ImGui::Begin(obj_name.data());
    {
        ImGui::Separator();
        if(ImGui::TreeNode(u8"SpringArm")) {
            u32* bit = &spring_arm_status_.get();
            u32  val = *bit;
            ImGui::CheckboxFlags(u8"初期化済", &val, 1 << (int)SpringArmBit::Initialized);

            ImGui::DragFloat3(u8"SpringArm座標", (float*)&spring_arm_vector_, 0.1f, -10000.0f, 10000.0f, "%.1f");
            ImGui::DragFloat3(u8"SpringArmオフセット", (float*)&spring_arm_offset_, 0.1f, -10000.0f, 10000.0f, "%.1f");

            ImGui::DragFloat(u8"SpringArm 固さ", (float*)&spring_arm_strong_, 0.1f, 0.0f, 1.0f);
            ImGui::DragFloat(u8"SpringArm 戻り", (float*)&spring_arm_return_, 0.1f, 0.0f, 1.0f);

            ImGui::TreePop();
        }
    }
    ImGui::End();
}
