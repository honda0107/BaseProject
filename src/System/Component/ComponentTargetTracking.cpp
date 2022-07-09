#include <System/Component/ComponentTargetTracking.h>
#include <System/Component/ComponentModel.h>
#include <System/Component/ComponentTransform.h>
#include <System/Object.h>
#include <System/Scene.h>

#include <System/ImGui.h>
#include <System/Utils/HelperLib.h>

namespace
{
std::string null_name = "  ";

// string用のCombo
static int Combo(const std::string& caption, std::string& current_item, const std::vector<std::string_view>& items)
{
    int select_index = -1;

    if(ImGui::BeginCombo(caption.c_str(), current_item.c_str())) {
        for(int i = 0; i < items.size(); i++) {
            auto& item = items[i];

            bool is_selected = (current_item == item);
            if(ImGui::Selectable(item.data(), is_selected)) {
                current_item = item;
                select_index = i;
            }
            if(is_selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    return select_index;
}

#if 0   // DecomposeMatrixToComponentsを使って角度を求めるためこのルーチンはOFFにした
	//! @brief 指定マトリクスの軸にあった法線(ポイント)を求めます
	//! @param mat 指定のマトリクス
	//! @param point 点
	//! @param xz_vec xz軸面(地面)ベクトル
	//! @param yz_vec yz軸面(視線)ベクトル
	//! @param xy_vec xy軸面(画面)ベクトル
	void CalculateNormalWithEachAxis( matrix mat, float3 point, float3* xz_vec, float3* yz_vec, float3* xy_vec )
	{
		float3 front = mat.axisZ();
		float3 right = mat.axisX();
		float3 up	 = mat.axisY();

		float3 front_point = point - mat.translate();
		front_point		   = normalize( point - mat.translate() );
		// 真上の時
		if( dot( front_point, up ).x > 1.0f - FLT_EPSILON )
		{
			// 30m上空を見るようにする
			front_point	  = { mat.axisZ() };
			front_point.y = 30.0;
			front_point	  = normalize( front_point );
		}
		// 真下のとき
		if( dot( front_point, up ).x < -1.0f + FLT_EPSILON )
		{
			// 30mしたを見るようにする
			front_point	  = { mat.axisZ() };
			front_point.y = -30.0;
			front_point	  = normalize( front_point );
		}

		right = cross( up, front_point );

		// 地面での向き
		*xz_vec = normalize( cross( right, up ) );

		right = mat.axisX();

		// ほぼ右を見ているとき
		if( dot( front_point, right ).x > 1.0f - FLT_EPSILON )
		{
			*yz_vec = float3{ 0.0f, 0.0f, 0.0f };
		}
		if( dot( front_point, right ).x < -1.0f + FLT_EPSILON )
		{
			*yz_vec = float3{ 0.0f, 0.0f, 0.0f };
		}
		else
		{
			up = normalize( cross( front_point, right ) );
			// 上下の向き
			*yz_vec = normalize( cross( right, up ) );
		}

		up = cross( front, front_point );

		*xy_vec = normalize( cross( front, up ) );
	}
#endif

}   // namespace

//---------------------------------------------------------
//! カメラ初期化
//---------------------------------------------------------
void ComponentTargetTracking::Init()
{
    __super::Init();

    tracking_status_.on(TrackingBit::Initialized);
}

//---------------------------------------------------------
//! 更新
//---------------------------------------------------------
void ComponentTargetTracking::PreUpdate()
{
#if 1
    if(auto model = owner_model_.lock()) {
        MV1SetFrameUserLocalMatrix(model->GetModel(), tracked_node_index_, cast(tracking_matrix_));
    }
#endif
}

void ComponentTargetTracking::PostUpdate()
{
    if(tracked_node_index_ < 0)
        return;

    float3 target_pos = look_at_;

    if(auto target = tracking_object_.lock())
        target_pos = target->GetTranslate();

    if(owner_model_.lock() == nullptr)
        owner_model_ = GetOwner()->GetComponent<ComponentModel>();

    if(auto model = owner_model_.lock()) {
        float3 up = model->GetVectorAxisY();

        float3 model_pos = model->GetTranslate() + tracking_matrix_._41_42_43;

        model_pos       = mul(float4(model_pos, 1), GetOwner()->GetMatrix()).xyz;
        float3 model_up = mul(float4(model->GetVectorAxisY(), 0), GetOwner()->GetMatrix()).xyz;

        auto fmat = inverse(HelperLib::Math::CreateMatrixByFrontVector(front_vector_));

        float3 vec    = target_pos - model_pos;
        auto   mat    = HelperLib::Math::CreateMatrixByFrontVector(vec, model_up, true);
        mat._41_42_43 = model_pos;

        mat = mul(fmat, mat);

        float  theta_xz = 0.0f;
        float  theta_yz = 0.0f;
        float3 rot;
        float3 trans;
        float3 scale;

        DecomposeMatrixToComponents((float*)mat.f32_128_0, (float*)&trans, (float*)&rot, (float*)&scale);

        if(rot.y > 180)
            rot.y = rot.y - 360;
        if(rot.x < -180)
            rot.x += 360;
        theta_xz = rot.y;
        theta_yz = -rot.x;

        // 右リミット
        if(theta_xz > limit_lr_.y) {
            theta_xz = limit_lr_.y;
        }
        // 左リミット
        if(theta_xz < -(float)limit_lr_.x) {
            theta_xz = -limit_lr_.x;
        }

        // 上リミット
        if(theta_yz > limit_ud_.x) {
            theta_yz = limit_ud_.x;
        }
        // 下リミット
        if(theta_yz < -(float)limit_ud_.y) {
            theta_yz = -limit_ud_.y;
        }

        auto roty = matrix::rotateY(radians((float1)theta_xz));
        auto rotx = matrix::rotateX(radians((float1)theta_yz));

        mat = mul(mul(tracking_matrix_, rotx), roty);

        MV1SetFrameUserLocalMatrix(model->GetModel(), tracked_node_index_, cast(mat));
    }
}

//---------------------------------------------------------
//! 向きのマトリクス表示(デバッグ用)
//---------------------------------------------------------
void ComponentTargetTracking::Draw()
{
}

//---------------------------------------------------------
//! カメラ終了処理
//---------------------------------------------------------
void ComponentTargetTracking::Exit()
{
    __super::Exit();
}

//---------------------------------------------------------
//! カメラGUI処理
//---------------------------------------------------------
void ComponentTargetTracking::GUI()
{
    assert(GetOwner());
    auto obj_name = GetOwner()->GetName();

    ImGui::Begin(obj_name.data());
    {
        ImGui::Separator();
        if(ImGui::TreeNode(u8"TargetTracking")) {
            int select_node_index = -1;
            if(auto model = GetOwner()->GetComponent<ComponentModel>()) {
                auto list         = model->GetNodesName();
                select_node_index = Combo(u8"トラッキングノード", tracked_node_, list);
                if(select_node_index < list.size())
                    SetTrackingNode(select_node_index);
            }

            if(auto scene = Scene::GetCurrentScene()) {
                std::string name = "";
                if(auto obj = tracking_object_.lock()) {
                    name = obj->GetName();
                }

                auto                          objs = scene->GetObjectPtrVec();
                std::vector<std::string_view> names;
                names.push_back(null_name);
                for(auto obj : objs) {
                    names.push_back(obj->GetName());
                }
                int id = Combo(u8"ターゲットオブジェクト", name, names) - 1;

                if(id >= 0) {
                    tracking_object_ = objs[id];
                }
                else if(id == -1) {
                    tracking_object_.reset();
                }
            }

            tracking_status_.off(TrackingBit::ObjectTracking);
            if(auto obj = tracking_object_.lock()) {
                tracking_status_.on(TrackingBit::ObjectTracking);
            }

            if(!tracking_status_.is(TrackingBit::ObjectTracking)) {
                ImGui::DragFloat3(u8"トラッキングポイント", (float*)&look_at_, 0.01f);
            }

            ImGui::Separator();
            ImGui::DragFloat2(u8"左右角度制限", (float*)&limit_lr_, 0.1f, 0, 180, "%.1f");
            ImGui::DragFloat2(u8"上下角度制限", (float*)&limit_ud_, 0.1f, 0, 180, "%.1f");
            ImGui::Separator();
            ImGui::DragFloat3(u8"前ベクトル", (float*)&front_vector_, 0.1f, 0, 180, "%.1f");

            ImGui::TreePop();
        }
    }
    ImGui::End();
}

void ComponentTargetTracking::SetTargetObjectPtr(ObjectWeakPtr obj)
{
    tracking_object_ = obj;
    tracking_status_.on(TrackingBit::ObjectTracking);
}

void ComponentTargetTracking::SetTargetDirection(float3 target)
{
    tracking_object_.reset();
    look_at_ = target;
    tracking_status_.off(TrackingBit::ObjectTracking);
}

void ComponentTargetTracking::SetTrackingNode(const std::string& name)
{
    if(owner_model_.lock() == nullptr)
        owner_model_ = GetOwner()->GetComponent<ComponentModel>();

    if(auto model = owner_model_.lock()) {
        tracked_node_ = name;

        tracked_node_index_ = MV1SearchFrame(model->GetModel(), name.c_str());

        auto mat         = cast(MV1GetFrameLocalMatrix(model->GetModel(), tracked_node_index_));
        tracking_matrix_ = mat;
    }
}

void ComponentTargetTracking::SetTrackingNode(int tracking_node_index)
{
    tracked_node_index_ = tracking_node_index;
}

void ComponentTargetTracking::SetTrackingLimitLeftRight(float2 left_right)
{
    limit_lr_ = left_right;
}

void ComponentTargetTracking::SetTrackingLimitUpDown(float2 up_down)
{
    limit_ud_ = up_down;
}

void ComponentTargetTracking::SetFrontVector(float3 vec)
{
    front_vector_ = normalize(vec);
}
