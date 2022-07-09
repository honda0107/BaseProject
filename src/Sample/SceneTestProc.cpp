#include "SceneTestProc.h"
#include <System/Component/ComponentModel.h>

BP_CLASS_IMPL(SceneTestProc, u8"プロセステスト");

bool SceneTestProc::Init()
{
    auto obj = Scene::CreateObject<Object>();
    auto mdl = obj->AddComponent<ComponentModel>("data/Sample/Player/model.mv1")->SetScaleAxisXYZ({0.05f});

    return true;
}

// 標準以外の処理はセーブ・ロードができないため、
// ラムダ式などを追加する場合は、InitSerialize()を利用するのが良い。
void SceneTestProc::InitSerialize()
{
    // シーンにあるオブジェクトを取得
    auto obj = Scene::GetObjectPtr<Object>();

    // Drawという名前で、Drawのタイミングにて処理追加
    obj->SetProc("Draw", []() { printfDx("Draw!\n"); });

    // Updateという名前で、Updateのタイミングにて処理追加 (引数をfloatにするとUpdateに追加)
    obj->SetProc("Update", [](float) { printfDx("Update!\n"); });

    // ABCという名前で、PreDrawのタイミングに処理追加
    obj->SetProc(
        "ABC",
        []() { printfDx("PreDraw!\n"); },
        ProcTiming::PreDraw);

    // Testという名前で、Updateタイミングに追加 (優先付き)
    obj->SetProc(
        "Test",
        [](float) { printfDx("Test!\n"); },
        ProcTiming::Update,
        Priority::NORMAL);

    // ライトのタイミングでプロセスを発生 (今回追加した Lightタイミング )
    // ※ Light/HDR/Gbuffer/Shadow は、現在適当なタイミングで用意しています
    // Scene::Draw内の　プロセスシグナルの実行　にて実行
    // ※ 調整が必要となります
    obj->SetProc(
        "CDE",
        []() { printfDx("Light\n"); },
        ProcTiming::Light,
        Priority::NORMAL);

    // コンポーネントに処理をつける
    if(auto mdl = obj->GetComponent<ComponentModel>())
        mdl->SetProc("CompUpdate", [](float) { printfDx("CompUpdate!\n"); });

    // 注意点は、lamdaに投げるオブジェクトはweak_ptrにする必要がある。
    // 使用しない場合、最後までCaptureし続け、DxLib_End内でエラーとなる
    std::weak_ptr<Object> wkobj = obj;

    // Updateを違う処理に変更します
    obj->SetProc("Update", [wkobj](float) {
        printfDx("Update!\n");
        if(auto obj = wkobj.lock())
            obj->AddRotationAxisXYZ({0, 1, 0});
    });
}

void SceneTestProc::Update([[maybe_unused]] float delta)
{
    // カメラの設定
    SetCameraPositionAndTarget_UpVecY({0.f, 6.f, -35.f}, {0.f, 1.f, 0.f});
    SetupCamera_Perspective(60.0f * DegToRad);

    // スペースを押したら次に行く
    if(IsKeyOn(KEY_INPUT_SPACE)) {
        auto obj = GetObjectPtr<Object>();

        // 動作追加
        obj->SetProc("Update2", [](float) { printfDx("Update2!\n"); });

        // 現在 Update(引数あり)と無しとでは名前共有ができていません。
        // 今後共有予定です。(LightタイミングのCDEは現在消しません)
        obj->SetProc<float>(
            "CDE",
            [](float) { printfDx("Update3!\n"); },
            ProcTiming::Update,
            Priority::NORMAL);

        // ABCのプロセスを終了します (PreDrawのものを終了)
        obj->ResetProc("ABC");

        // Updateを消します
        obj->ResetProc("Update");

        // コンポーネントの処理を削除する
        if(auto mdl = obj->GetComponent<ComponentModel>()) {
            mdl->ResetProc("CompUpdate");
        }
    }
}

void SceneTestProc::Draw()
{
    // とりあえずTitleという文字を表示しておく
    DrawFormatString(100, 50, GetColor(255, 255, 255), "Title");
}

void SceneTestProc::Exit()
{
    // タイトル終了時に行いたいことは今はない
}

void SceneTestProc::GUI()
{
}
