//---------------------------------------------------------------------------
//!	@file	SystemMain.cpp
//! @brief	システムメイン
//---------------------------------------------------------------------------
#include <System/Debug/DebugCamera.h>
#include <System/Physics/PhysicsEngine.h>

//----------------------------------------------------------------
// シーンオブジェクト
//----------------------------------------------------------------
#include "SceneSample.h"
#include <System/Scene.h>
#include <System/Utils/IniFileLib.h>

namespace
{

bool show_gui     = true;    //!< GUIの表示
bool show_grid    = true;    //!< グリッドの表示
bool show_fps     = true;    //!< FPSの表示
bool debug_camera = false;   //!< デバッグカメラ
bool show_debug   = true;

u64 current_time_ = 0;      //!< 現在の時間 (単位:μsec)
f32 delta_time_   = 0.0f;   //!< 1フレームの経過時間（CPUとGPU, ScreenFlip()更新待ちすべて含む）

bool menu_active = false;
bool menu_select = false;

//--------------------------------------------------------------
//! @name   CPU負荷計測
//--------------------------------------------------------------
//@{

u64 cpu_start_counter_    = 0;   //!< 1フレームの開始タイミングカウンター
u64 cpu_profile_duration_ = 0;   //!< 1フレームのCPU処理時間(単位:μsec)

//@}

//! 物理シミュレーションの実体
std::unique_ptr<physics::Engine> physicsEngine_;

// デバッグカメラの移動方法
DebugCamera::Control control = DebugCamera::UnrealEngine;

}   // namespace

//---------------------------------------------------------------------------
//! 1フレームのCPU処理時間を取得(単位:μsec)
//---------------------------------------------------------------------------
u64 GetCpuProfile()
{
    return cpu_profile_duration_;
}

//---------------------------------------------------------------------------
//! パフォーマンスカウンターを取得 (単位:μsec)
//---------------------------------------------------------------------------
u64 GetPerformanceCounterMicroSec()
{
    u64 counter = GetNowSysPerformanceCount();
    return ConvSysPerformanceCountToMicroSeconds(counter);
}

//---------------------------------------------------------------------------
//! 経過時間計測をリセット
//---------------------------------------------------------------------------
void ResetDeltaTime()
{
    current_time_ = GetPerformanceCounterMicroSec();
}

//---------------------------------------------------------------------------
//! 1フレームの間に経過した時間を計算
//---------------------------------------------------------------------------
void UpdateDeltaTime()
{
    u64 last_time = current_time_;
    current_time_ = GetPerformanceCounterMicroSec();
    delta_time_   = static_cast<f32>(current_time_ - last_time) * (1.0f / 1000.0f / 1000.0f);
}

//---------------------------------------------------------------------------------
//! グリッドの表示をON/OFFします
//---------------------------------------------------------------------------------
void ShowGrid(bool active)
{
    show_grid = active;
}

//===========================================================================
// 数値スクロール表示用のリングバッファ
//===========================================================================
struct ScrollingBuffer
{
    u32                 offset_ = 0;   //!< データー先頭の位置
    std::vector<ImVec2> data_;         //!< プロットデーター配列

    ScrollingBuffer(size_t max_size = 4096) { data_.reserve(max_size); }
    void AddPoint(f32 t, f32 value)
    {
        // プロットする値
        auto x = ImVec2(t, value);

        if(data_.size() < data_.capacity()) {
            // 新規追加
            data_.emplace_back(std::move(x));
        }
        else {
            // リングバッファとして再利用
            data_[offset_] = x;
            offset_        = (offset_ + 1) % static_cast<u32>(data_.capacity());
        }
    }
    void clear()
    {
        data_.clear();
        offset_ = 0;
    }
};

//---------------------------------------------------------------------------
//! FPSを表示します
//! @param  [in]    delta   1フレームの経過時間(単位:秒)
//---------------------------------------------------------------------------
void ShowFps(f32 delta)
{
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration |   // タイトルバー/スクロールバーなし
                                    ImGuiWindowFlags_NoDocking |      // ドッキングなし
                                    ImGuiWindowFlags_AlwaysAutoResize |     // 自動リサイズなし
                                    ImGuiWindowFlags_NoSavedSettings |      // 保存しない
                                    ImGuiWindowFlags_NoFocusOnAppearing |   // フォーカスしない
                                    ImGuiWindowFlags_NoNav |   // キーやゲームパッドで操作対象にしない
                                    ImGuiWindowFlags_NoMove;   // 移動させない

    //----------------------------------------------------------
    // 表示位置設定
    //----------------------------------------------------------
    constexpr u32 corner = 2;   // 0:左上 1:右上 2:左下 3: 右下
    {
        constexpr f32        PADDING   = 10.0f;   // 余白部分
        const ImGuiViewport* viewport  = ImGui::GetMainViewport();
        ImVec2               work_pos  = viewport->WorkPos;   // WorkPosを使用するとメニューバーなどの位置を考慮した位置が取得できる
        ImVec2               work_size = viewport->WorkSize;
        ImVec2               window_pos, window_pos_pivot;
        window_pos.x       = (corner & 1) ? (work_pos.x + work_size.x - PADDING) : (work_pos.x + PADDING);
        window_pos.y       = (corner & 2) ? (work_pos.y + work_size.y - PADDING) : (work_pos.y + PADDING);
        window_pos_pivot.x = (corner & 1) ? 1.0f : 0.0f;
        window_pos_pivot.y = (corner & 2) ? 1.0f : 0.0f;
        ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
        ImGui::SetNextWindowViewport(viewport->ID);
    }

    //----------------------------------------------------------
    // 半透明の背景
    //----------------------------------------------------------
    ImGui::SetNextWindowBgAlpha(0.35f);

    //----------------------------------------------------------
    // オーバーレイウィンドウを表示
    //----------------------------------------------------------

    // モニターのリフレッシュレートを取得 (Hz)
    s32 refresh_rate = 60;
    {
        HDC hdc      = GetDC(GetMainWindowHandle());   // デバイスコンテキストの取得
        refresh_rate = GetDeviceCaps(hdc, VREFRESH);   // リフレッシュレートの取得
        ReleaseDC(GetMainWindowHandle(), hdc);         // デバイスコンテキストの解放
    }

    // 現在のフレームレート
    auto frame_rate = ImGui::GetIO().Framerate;

    if(static_cast<f32>(refresh_rate) < ImGui::GetIO().Framerate) {
        frame_rate = static_cast<f32>(refresh_rate);
    }

    // CPU負荷(μsec)
    u64 cpu_profile_time = GetCpuProfile();

    // 角を丸める
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f);

    if(ImGui::Begin(u8"FPS計測", nullptr, window_flags)) {
        //----------------------------------------------------------
        // フレームレートグラフを表示
        //----------------------------------------------------------
        static ScrollingBuffer cpu_data, fps_data;

        // 経過時間
        static float t = 0;
        t += delta;

        // 数値を0.0f～1.0fの範囲で設定する
        f32  cpu_micro_sec = static_cast<f32>(cpu_profile_time);
        auto ratio         = cpu_micro_sec / (1000.0f * 1000.0f / static_cast<f32>(refresh_rate));

        cpu_data.AddPoint(t, ratio);                                         // CPU負荷
        fps_data.AddPoint(t, frame_rate / static_cast<f32>(refresh_rate));   // フレームレート

        static float history = 10.0f;

        constexpr ImPlotFlags flags = ImPlotFlags_NoInputs | ImPlotFlags_NoFrame;

        if(ImPlot::BeginPlot(u8"フレームレート & CPU負荷", ImVec2(-1.0f, 96.0f), flags)) {
            constexpr ImPlotAxisFlags axis_flags = ImPlotAxisFlags_NoTickLabels | ImPlotAxisFlags_Lock;

            ImPlot::SetupAxes(NULL, NULL, flags, axis_flags);
            ImPlot::SetupAxisLimits(ImAxis_X1, t - history, t, ImGuiCond_Always);   // 表示範囲
            ImPlot::SetupAxisLimits(ImAxis_Y1, 0.0f, 1.01f);   // 上下数値の範囲(最大値目盛りを出すため1.01f)
            ImPlot::SetNextFillStyle(IMPLOT_AUTO_COL, 0.5f);

            ImPlot::PlotShaded(u8"CPU負荷",                               // 名前
                               &cpu_data.data_[0].x,                      // 時間軸t
                               &cpu_data.data_[0].y,                      // 値
                               static_cast<s32>(cpu_data.data_.size()),   // 配列数
                               -INFINITY,                                 // 塗りつぶし範囲
                               cpu_data.offset_,                          // 先頭オフセット
                               sizeof(ImVec2));                           // 構造体あたりのサイズ

            ImPlot::PlotLine("fps",                                     // 名前
                             &fps_data.data_[0].x,                      // 時間軸t
                             &fps_data.data_[0].y,                      // 値
                             static_cast<s32>(fps_data.data_.size()),   // 配列数
                             fps_data.offset_,                          // 先頭オフセット
                             sizeof(ImVec2));                           // 構造体あたりのサイズ
            ImPlot::EndPlot();
        }
        ImGui::SliderFloat(u8"履歴範囲", &history, 1, 30, "%.1f s");
    }

    //----------------------------------------------------------
    // 現在の数値を表示
    //----------------------------------------------------------
    ImGui::Separator();
    ImGui::Text(u8"FPS    : %3.2f fps (max:%3d fps)", frame_rate, refresh_rate);
    ImGui::Text(u8"CPU負荷 : %3.2f ms", static_cast<f32>(cpu_profile_time) / 1000.0f);

    // オーバーレイウィンドウ終了
    ImGui::End();
    ImGui::PopStyleVar();   // 角を丸める設定を元に戻す
}

//---------------------------------------------------------------------------------
//! 初期化
//---------------------------------------------------------------------------------
void SystemInit()
{
    // Editor.iniから読み取り
    Scene::LoadEditor();

    // Game.iniから読み込む
    IniFileLib ini("Game.ini");

    auto scene_name = ini.GetString("Scene", "Start");
    // 作成
    auto* scene = CreateInstanceFromName<Scene::Base>(scene_name);
    if(scene) {
        Scene::Change(std::shared_ptr<Scene::Base>(scene));
    }
    else {
        // iniファイルの設定のクラスが見つからない場合はサンプルシーンを起動
        Scene::Change(std::make_shared<class SceneSample>());
    }

    //----------------------------------------------------------
    // 物理シミュレーションを初期化
    //----------------------------------------------------------
    physicsEngine_ = physics::createPhysics();

    // 現在の時間を初期化
    ResetDeltaTime();
}

//---------------------------------------------------------------------------------
//! 更新
//---------------------------------------------------------------------------------
void SystemUpdate()
{
    // 1フレームの間に経過した時間を計算 ⊿t
    UpdateDeltaTime();

    //----------------------------------------------------------
    // メインメニューバー
    //----------------------------------------------------------
    if(IsKeyOn(KEY_INPUT_LALT) || IsKeyOn(KEY_INPUT_RALT)) {
        menu_active = !menu_active;
    }

    // F5キーでデバッグ表示変更
    if(IsKeyOn(KEY_INPUT_F5)) {
        show_debug = !show_debug;
        Scene::SetEdit(show_debug);
    }
    // F4キーでカメラ変更
    if(IsKeyOn(KEY_INPUT_F4)) {
        debug_camera = !debug_camera;
        DebugCamera::Use(debug_camera);
    }

    menu_select = false;
    if(menu_active) {
        if(ImGui::BeginMainMenuBar()) {
            if(ImGui::BeginMenu("File")) {
                ImGui::EndMenu();
            }

            if(ImGui::BeginMenu("Edit")) {
                ImGui::EndMenu();
            }

            if(ImGui::BeginMenu("Scene")) {
                menu_select = true;
                if(ImGui::BeginMenu(u8"シーン選択")) {
                    //------------------------------------------
                    // 登録されているシーンを列挙する
                    //------------------------------------------
                    auto& scene_base = Scene::Base::TypeInfo;
                    for(auto* p = scene_base.child(); p; p = p->siblings()) {
                        // 表示文字列
                        std::string scene_name = std::string(p->className()) + " - " + p->descName();

                        // 現在のシーンなら選択状態にする
                        bool selected = false;
                        if(auto* current_scene = Scene::GetCurrentScene()) {
                            selected = (current_scene->typeInfo() == p);
                        }

                        if(ImGui::MenuItem(scene_name.c_str(), nullptr, selected)) {
                            if(!selected) {   // 現在のシーン以外が新たに選択された場合

                                auto* scene = reinterpret_cast<Scene::Base*>(p->createInstance());
                                // シーンジャンプ
                                if(scene) {
                                    // デバッグカメラはOFFにしてからシーン切り替えを行います
                                    debug_camera = false;
                                    DebugCamera::Use(debug_camera);
                                    Scene::Change(std::shared_ptr<Scene::Base>(scene));
                                }
                            }
                        }
                    }

                    ImGui::EndMenu();
                }

                ImGui::EndMenu();
            }

            if(ImGui::BeginMenu("Debug")) {
                menu_select = true;
                ImGui::Checkbox(u8"グリッド表示", &show_grid);
                ImGui::Checkbox(u8"FPS表示", &show_fps);
                ImGui::Separator();
                ImGui::Checkbox(u8"デバッグ表示(F5)", &show_debug);
                ImGui::Separator();
                ImGui::Checkbox(u8"デバッグカメラ(F4)", &debug_camera);
                if(debug_camera) {
                    if(ImGui::BeginMenu(u8"移動方式")) {
                        ImGui::RadioButton("Unity", (int*)(&control), DebugCamera::Unity);
                        ImGui::RadioButton("UnrealEngine", (int*)(&control), DebugCamera::UnrealEngine);
                        ImGui::RadioButton("Maya", (int*)(&control), DebugCamera::Maya);
                        ImGui::EndMenu();
                    }
                }
                DebugCamera::Use(debug_camera);
                DebugCamera::SetControl(control);
                ImGui::Separator();
                Scene::SetEdit(show_debug);
                ImGui::Separator();
                ImGui::CheckboxFlags(u8"エディター配置",
                                     (int*)&Scene::SceneStatus(),
                                     1 << (int)Scene::EditorStatusBit::EditorPlacement);
                if(Scene::GetEditorStatus(Scene::EditorStatusBit::EditorPlacement)) {
                    ImGui::CheckboxFlags(u8"Always",
                                         (int*)&Scene::SceneStatus(),
                                         1 << (int)Scene::EditorStatusBit::EditorPlacement_Always);
                }

                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }
    }

    //----------------------------------------------------------
    // シーンの更新前処理
    //----------------------------------------------------------
    Scene::PreUpdate();

    //----------------------------------------------------------
    // シーンの更新
    //----------------------------------------------------------
    Scene::Update(delta_time_);

    //----------------------------------------------------------
    // GUI処理&描画
    //----------------------------------------------------------
    if(show_gui)
        Scene::GUI();

    //----------------------------------------------------------
    // 物理シミュレーション前の処理
    //----------------------------------------------------------
    Scene::PrePhysics();

    //----------------------------------------------------------
    // 物理シミュレーションを更新
    //----------------------------------------------------------
    physicsEngine_->update(delta_time_);

    //----------------------------------------------------------
    // シーンの更新後処理
    //----------------------------------------------------------
    Scene::PostUpdate();
}

//---------------------------------------------------------------------------------
//! 描画
//---------------------------------------------------------------------------------
void SystemDraw()
{
    //----------------------------------------------------------
    // グリッドを描画
    //----------------------------------------------------------
    if(show_grid) {
        constexpr f32 size = 64.0f;   // グリッドの範囲

        for(f32 x = -size; x <= size; x += 1.0f) {
            DrawLine3D(VGet(x, 0.0f, -size), VGet(x, 0.0f, +size), GetColor(224, 224, 224));
        }
        for(f32 z = -size; z <= size; z += 1.0f) {
            DrawLine3D(VGet(-size, 0.0f, z), VGet(+size, 0.0f, z), GetColor(224, 224, 224));
        }

        // X軸
        DrawLine3D(VGet(-size, 0.0f, 0.0f), VGet(+size, 0.0f, 0.0f), GetColor(255, 64, 64));
        // Y軸
        DrawLine3D(VGet(0.0f, -size, 0.0f), VGet(0.0f, +size, 0.0f), GetColor(64, 255, 64));
        // Z軸
        DrawLine3D(VGet(0.0f, 0.0f, -size), VGet(0.0f, 0.0f, +size), GetColor(64, 64, 255));
    }

    // シーンの描画
    Scene::Draw();

    // FPSの表示
    if(show_fps) {
        ShowFps(delta_time_);
    }
}

//---------------------------------------------------------------------------------
//! 終了
//---------------------------------------------------------------------------------
void SystemExit()
{
    Scene::SaveEditor();

    Scene::Exit();

    //----------------------------------------------------------
    // 物理シミュレーションを解放
    //----------------------------------------------------------
    physicsEngine_.reset();
}

//---------------------------------------------------------------------------------
//	フレームの開始
//---------------------------------------------------------------------------------
void SystemBeginFrame()
{
    // CPU計測開始
    cpu_start_counter_ = GetPerformanceCounterMicroSec();
}

//---------------------------------------------------------------------------------
//	フレームの終了
//---------------------------------------------------------------------------------
void SystemEndFrame()
{
    // CPU計測終了
    u64 cpu_end_counter   = GetPerformanceCounterMicroSec();
    cpu_profile_duration_ = cpu_end_counter - cpu_start_counter_;
}

bool IsShowMenu()
{
    return menu_active && menu_select;
}
