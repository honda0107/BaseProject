//---------------------------------------------------------------------------
//! @file   SceneRender.cpp
//! @brief  サンプルシーン
//---------------------------------------------------------------------------
#include "SceneRender.h"
#include <System/Component/ComponentModel.h>
#include <System/Component/ComponentCamera.h>

BP_CLASS_IMPL(SceneRender, u8"レンダリングシーン")

//---------------------------------------------------------------------------
//! 初期化
//---------------------------------------------------------------------------
bool SceneRender::Init()
{
    //----------------------------------------------
    //  オブジェクトの作成
    //----------------------------------------------
    auto object = Scene::CreateObject<Object>();

    //  モデルコンポーネント
    model_ = object->AddComponent<ComponentModel>("data/Sample/Player/model.mv1");

    //  カメラコンポーネント
    camera_ = object->AddComponent<ComponentCamera>();
    camera_->SetPerspective(60.0f);

    float3 eye     = float3(15.0f, 6.0f, -15.0f);
    float3 look_at = float3(0.0f, 5.0f, 0.0f);

    camera_->SetPositionAndTarget(eye, look_at);

    // 仮モデルの読み込み
    //model_ = MV1LoadModel("data/Sample/Player/model.mv1");

    return true;
}

//---------------------------------------------------------------------------
//! 更新
//! @param  [in]    delta   経過時間
//---------------------------------------------------------------------------
void SceneRender::Update([[maybe_unused]] float delta)
{
    float3 move = float3(0.0f, 0.0f, 0.0f);

    if(IsKeyRepeat(KEY_INPUT_LEFT)) {
        move.x -= 1.0f;
    }
    if(IsKeyRepeat(KEY_INPUT_RIGHT)) {
        move.x += 1.0f;
    }
    if(IsKeyRepeat(KEY_INPUT_UP)) {
        move.z += 1.0f;
    }
    if(IsKeyRepeat(KEY_INPUT_DOWN)) {
        move.z -= 1.0f;
    }

    //  長さの二乗0でなければ正規化
    if(dot(move, move).x != 0.0f) {
        move = normalize(move);
    }

    //  移動
    position_ += move * 0.1f;

    // 仮モデルの設定
    //MV1SetPosition(model_, cast(position_));
    //MV1SetRotationXYZ(model_, {0.0f, 0.0f, 0.0f});
    //MV1SetScale(model_, {0.05f, 0.05f, 0.05f});

    // カメラの設定
    VECTOR eye     = {15.0f, 10.0f, -15.0f};   //  目の位置
    VECTOR look_at = {0.0f, 5.0f, 0.0f};       //  注目点/注視点

    SetCameraPositionAndTarget_UpVecY(eye, look_at);
    SetupCamera_Perspective(60.0f * DX_PI_F / 180.0f);
}

//---------------------------------------------------------------------------
//! 描画
//---------------------------------------------------------------------------
void SceneRender::Draw()
{
    // 仮モデルの描画
    //MV1DrawModel(model_);

    //--------------------------------------------------------
    //  行列の親子関係のテスト
    //--------------------------------------------------------
    {
        static u32 counter = 0;
        counter++;

        static float3 p(0.0f, 0.0f, 0.0f);
        p.z += 0.02f;

        //  原点を中心に回転させる
        matrix mat_sun   = matrix::translate(p);
        matrix mat_earth = mul(matrix::translate(5.0f, 0.0f, 0.0f), matrix::rotateY(counter * 0.01f));
        matrix mat_moon  = mul(matrix::translate(1.5f, 0.0f, 0.0f), matrix::rotateY(counter * 0.05f));

        //  親子関係を設定
        //  [右側から親の行列をかける]
        mat_earth = mul(mat_earth, mat_sun);
        mat_moon  = mul(mat_moon, mat_earth);

        //  位置座標
        float3 position_sum   = mat_sun.translate();   //  matrixから平行ベクトルを取得
        float3 position_earth = mat_earth.translate();
        float3 position_moon  = mat_moon.translate();

        SetUseLighting(false);   //  ライティングOFF
        {
            DrawSphere3D(cast(position_sum), 1.0f, 8, GetColor(255, 0, 0), 0, false);

            DrawSphere3D(cast(position_earth), 0.1f, 8, GetColor(0, 255, 255), 0, false);

            DrawSphere3D(cast(position_moon), 0.01f, 8, GetColor(0, 255, 255), 0, false);
        }
        SetUseLighting(true);
    }
}

//---------------------------------------------------------------------------
//! 終了
//---------------------------------------------------------------------------
void SceneRender::Exit()
{
    //MV1DeleteModel(model_);
}

//---------------------------------------------------------------------------
//! GUI表示
//---------------------------------------------------------------------------
void SceneRender::GUI()
{
}
