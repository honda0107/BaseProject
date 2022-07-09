//---------------------------------------------------------------------------
//! @file   ComponentTransform.cpp
//! @brief  トランスフォームコンポーネント(座標と姿勢と大きさ)
//---------------------------------------------------------------------------
#include <System/Component/ComponentTransform.h>
#include <System/Object.h>

#include <ImGuizmo/ImGuizmo.h>

Component* ComponentTransform::select_component_ = nullptr;

namespace
{
void showGizmo(float* matrix, ImGuizmo::OPERATION ope, ImGuizmo::MODE mode)
{
    // Gizmoを表示するためのMatrixをDxLibから取得
    auto camera_view = GetCameraViewMatrix();
    auto camera_proj = GetCameraProjectionMatrix();

    // Gizmoの表示を設定する
    ImGuizmo::SetOrthographic(false);
    ImGuizmo::SetDrawlist(ImGui::GetBackgroundDrawList());

    // 画面サイズを取得する
    RECT rect;
    RECT crect;
    GetWindowRect(GetMainWindowHandle(), &rect);
    GetClientRect(GetMainWindowHandle(), &crect);

    // Gizmoを画面に合わせて処理する
    float windowWidth  = (float)rect.right - rect.left;
    float windowHeight = (float)rect.bottom - rect.top;
    float windowBar    = windowHeight - crect.bottom;
    ImGuizmo::SetRect((float)rect.left, (float)rect.top + windowBar / 2, windowWidth, windowHeight - windowBar / 2);

    // Manipulateを表示する
    static bool  useSnap         = false;
    static float snap[3]         = {1.0f, 1.0f, 1.0f};
    static float bounds[]        = {-0.5f, -0.5f, -0.5f, 0.5f, 0.5f, 0.5f};
    static float boundsSnap[]    = {0.1f, 0.1f, 0.1f};
    static bool  boundSizing     = false;
    static bool  boundSizingSnap = false;
    ImGuizmo::SetID(0);
    ImGuizmo::AllowAxisFlip(false);   //< これがないとGizmoが反転してしまう
    ImGuizmo::Manipulate((const float*)&camera_view,
                         (const float*)&camera_proj,
                         ope,
                         mode,
                         (float*)matrix,
                         NULL,
                         false ? &snap[0] : NULL,
                         boundSizing ? bounds : NULL,
                         boundSizingSnap ? boundsSnap : NULL);
}
}   // namespace

// ImGuizmoのMatrixからEulerに変換する際に全軸に180度変換がかかってしまうため修正
void DecomposeMatrixToComponents(const float* matx, float* translation, float* rotation, float* scale)
{
    ImGuizmo::DecomposeMatrixToComponents(matx, translation, rotation, scale);
    if(abs(rotation[0]) + abs(rotation[1]) + abs(rotation[2]) > 120 + 120) {
        rotation[0] -= 180.0f;
        rotation[1] = -180.0f - rotation[1];
        rotation[2] -= 180.0f;
    }
    if(rotation[0] < -180.0f)
        rotation[0] += 360.0f;
    if(rotation[0] > 180.0f)
        rotation[0] -= 360.0f;

    if(rotation[1] < -180.0f)
        rotation[1] += 360.0f;
    if(rotation[1] > 180.0f)
        rotation[1] -= 360.0f;

    if(rotation[2] < -180.0f)
        rotation[2] += 360.0f;
    if(rotation[2] > 180.0f)
        rotation[2] -= 360.0f;
}

void ComponentTransform::PostUpdate()
{
    __super::PostUpdate();

    old_transform_ = GetWorldMatrix();
}

//! @brief GUI処理
void ComponentTransform::GUI()
{
    // オーナーの取得
    assert(GetOwner());
    auto obj_name = GetOwner()->GetName();

    // 自分が選択されていたらGUI処理する
    // 注意: 複数Gizmoを発生させると全部同じ所で処理されてしまう
    if(is_guizmo_) {
        // Gizmo表示
        showGizmo(GetMatrixFloat(), gizmo_operation_, gizmo_mode_);

        // キーにより、Manipulateの処理を変更する
        // TODO : 一旦UE4に合わせておくが、のちにEditor.iniで設定できるようにする
        // W = Translate / E = Rotate / R = Scale (Same UE5)
        if(ImGui::IsKeyPressed('W'))
            gizmo_operation_ = ImGuizmo::TRANSLATE;
        if(ImGui::IsKeyPressed('E'))
            gizmo_operation_ = ImGuizmo::ROTATE;
        if(ImGui::IsKeyPressed('R'))
            gizmo_operation_ = ImGuizmo::SCALE;
    }

    // GUI描画
    ImGui::Begin(obj_name.data());
    {
        ImGui::Separator();
        is_guizmo_ = false;
        if(ImGui::TreeNode("Transform")) {
            ImGui::DragFloat4(u8"Ｘ軸", VectorAxisXFloat(), 0.01f, -10000.0f, 10000.0f, "%.2f");
            ImGui::DragFloat4(u8"Ｙ軸", VectorAxisYFloat(), 0.01f, -10000.0f, 10000.0f, "%.2f");
            ImGui::DragFloat4(u8"Ｚ軸", VectorAxisZFloat(), 0.01f, -10000.0f, 10000.0f, "%.2f");
            ImGui::DragFloat4(u8"座標", TranslateFloat(), 0.01f, -10000.0f, 10000.0f, "%.2f");
            ImGui::Separator();
            ImGui::TreePop();
        }

        if(ImGui::IsWindowFocused()) {
            select_component_ = this;
        }
        if(select_component_ == this)
            is_guizmo_ = true;

        // ギズモの処理選択
        if(ImGui::RadioButton(u8"座標", gizmo_operation_ == ImGuizmo::TRANSLATE))
            gizmo_operation_ = ImGuizmo::TRANSLATE;
        ImGui::SameLine();
        if(ImGui::RadioButton(u8"回転", gizmo_operation_ == ImGuizmo::ROTATE))
            gizmo_operation_ = ImGuizmo::ROTATE;
        ImGui::SameLine();
        if(ImGui::RadioButton(u8"サイズ", gizmo_operation_ == ImGuizmo::SCALE))
            gizmo_operation_ = ImGuizmo::SCALE;
        ImGui::SameLine();
        if(ImGui::RadioButton(u8"全部", gizmo_operation_ == ImGuizmo::UNIVERSAL))
            gizmo_operation_ = ImGuizmo::UNIVERSAL;

        if(gizmo_operation_ != ImGuizmo::SCALE) {
            if(ImGui::RadioButton("Local", gizmo_mode_ == ImGuizmo::LOCAL))
                gizmo_mode_ = ImGuizmo::LOCAL;
            ImGui::SameLine();
            if(ImGui::RadioButton("World", gizmo_mode_ == ImGuizmo::WORLD))
                gizmo_mode_ = ImGuizmo::WORLD;
        }

        // TRSにてマトリクスを再度作成する
        float* mat = GetMatrixFloat();
        float  matrixTranslation[3], matrixRotation[3], matrixScale[3];
        DecomposeMatrixToComponents(mat, matrixTranslation, matrixRotation, matrixScale);
        ImGui::DragFloat3(u8"座標(T)", matrixTranslation);
        ImGui::DragFloat3(u8"回転(R)", matrixRotation);
        ImGui::DragFloat3(u8"サイズ(S)", matrixScale);
        ImGuizmo::RecomposeMatrixFromComponents(matrixTranslation, matrixRotation, matrixScale, mat);
    }
    ImGui::End();
}
