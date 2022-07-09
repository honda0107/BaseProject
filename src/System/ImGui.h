// ImGui
#include <imgui/misc/single_file/imgui_single_file.h>

// implot
#include <implot/implot.h>

//	初期化
void ImGuiInit();

//	更新
void ImGuiUpdate();

//	描画
void ImGuiDraw();

//	終了
void ImGuiExit();

// Destroyが早くリーク情報が出せないため、WindowをDestroyを遅らせる
bool IsProcEnd();
