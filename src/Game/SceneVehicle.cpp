//---------------------------------------------------------------------------
//! @file   SceneVehicle.cpp
//! @brief  自動車シーン
//---------------------------------------------------------------------------
#include "SceneVehicle.h"
#include <System/Component/ComponentModel.h>
#include <System/Component/ComponentCamera.h>

BP_CLASS_IMPL(SceneVehicle, u8"自動車サンプル")

//---------------------------------------------------------------------------
//! 初期化
//---------------------------------------------------------------------------
bool SceneVehicle::Init()
{
    //-------------------------------------------------
    // モデル
    //-------------------------------------------------
    model_body_ = std::make_shared<Model>("data/Game/Civic/body.mv1");

    for(auto& x : model_tire_) {
        x = std::make_shared<Model>("data/Game/Civic/tire.mv1");
    }
    //----------------------------------------------
    //  オブジェクトの作成
    //----------------------------------------------
    auto object = Scene::CreateObject<Object>();

    //  カメラコンポーネント
    camera_ = object->AddComponent<ComponentCamera>();
    camera_->SetPerspective(60.0f);

    float3 eye     = float3(10.0f, 3.0f, -10.0f);
    float3 look_at = float3(0.0f, 3.0f, 0.0f);

    camera_->SetPositionAndTarget(eye, look_at);

    return true;
}

//---------------------------------------------------------------------------
//! 更新
//! @param  [in]    delta   経過時間
//---------------------------------------------------------------------------
void SceneVehicle::Update([[maybe_unused]] float delta)
{
    //--------------------------------------------------------------
    //	アクセル・ブレーキ操作
    //--------------------------------------------------------------
    //	アクセル
    gas_pedal_ += IsKeyRepeat(KEY_INPUT_X) ? 0.05f : -0.05f;
    gas_pedal_ = std::clamp(gas_pedal_, 0.0f, 1.0f);

    //	ブレーキ
    brake_pedal_ += IsKeyRepeat(KEY_INPUT_Z) ? 0.1f : -0.1f;
    brake_pedal_ = std::clamp(brake_pedal_, 0.0f, 1.0f);

    //--------------------------------------------------------------
    //	ステアリング操作
    //--------------------------------------------------------------
    if(IsKeyRepeat(KEY_INPUT_LEFT)) {
        steering_ -= 0.05f;
    }
    if(IsKeyRepeat(KEY_INPUT_RIGHT)) {
        steering_ += 0.05f;
    }

    //	左右操作がないときはニュートラルに戻す
    if(!IsKeyRepeat(KEY_INPUT_LEFT) && !IsKeyRepeat(KEY_INPUT_RIGHT)) {
        steering_ *= 0.95f;
    }

    steering_ = std::clamp(steering_, -1.0f, +1.0f);

    //--------------------------------------------------------------
    //	挙動計算
    //--------------------------------------------------------------
    static float tire_rotate_;

    {
        float3 right_dir = mat_world_.axisX();   //	車の右方向ベクトル
        float3 front_dir = mat_world_.axisZ();   //	車の前方向ベクトル

        constexpr float mass = 1350.0f;   //	1.35t

        //	F = ma  →  a = F/m
        float force = gas_pedal_ * 8000.0f;   //	近似

        //	加速度[m/(s^2)]
        velocity_ += front_dir * (force / mass * delta);

        //	減速
        //velocity_ *= 0.99f;

        //	ブレーキ減速
        //	velocity_.z *= lerp(float1(0.95f), float1(0.99f), float1(brake_pedal_));

        //	速度を成分分解して、各方向に効果を適用
        f32 right = dot(velocity_, right_dir);
        f32 front = dot(velocity_, front_dir);

        right *= 0.1f;    //	タイヤグリップで横方向が減衰(場所によって変えると良い)
        front *= 0.99f;   //	転がり抵抗
        tire_rotate_ += front * 0.04f;

        //	速度を再構築
        velocity_ = right_dir * right + front_dir * front;

        //	可変フレームレート(左辺と右辺の単位が合っている)
        // position_[m] = position_[m] + (velocity_[m/s] * time[s])
        // [m] = [m] + [(m/s * s)]
        // [m] = [m] + [m]    ← 両辺で単位が同じになった
        //	移動[m] += 速度[m/s] * 時間[s]
        //	position_ += velocity_ * delta;
    }
    //----------------------------------------------
    // 挙動計算用のタイヤ位置を計算
    //----------------------------------------------
    //	四輪のタイヤの位置(ボディからの相対位置)
    static const std::array<float3, 4> tirePosition{
        float3(-0.9f, 0.15f, +1.6f),   // [0] 左前		[0] 目--目 [1]
        float3(+0.9f, 0.15f, +1.6f),   // [1] 右前			|	 |
        float3(-0.9f, 0.15f, -1.6f),   // [2] 左後			|	 |
        float3(+0.9f, 0.15f, -1.6f),   // [3] 右後		[2] 目--目 [3]
    };

    for(u32 i = 0; i < 4; i++) {
        //	タイヤのワールド位置を求める
        float3 tire = tirePosition[i];
        tire.y      = 0.0f;   //	Y方向の移動量を無効化(見た目の位置調整のための値のため)

        tire_world_position_[i] = mul(float4(tire, 1.0f), mat_world_).xyz;

        //	移動量
        //	[m] = [m/s] * [s]
        float3 move = velocity_ * delta;

        //	前輪の場合は回転角度を与える
        if((i >> 1) == 0)   //	前輪のみ
            move = mul(float4(move, 0.0f), matrix::rotateY(steering_ * 35.0f * 0.75f * DegToRad)).xyz;

        //		move = normalize(move);	   //	テストで一時的に長さ統一

        //	移動させる
        tire_world_position_[i] += move;
    }

    //	車体の方向を求める
    //		 → axis_x
    //	[0]+---------+[1]
    //	   |		 |
    //	   |		 |	↑ axis_z
    // 	   |		 |
    // 	   |		 |
    // 	   |		 |
    //	[2]+---------+[3]
    //
    {
        auto&  tire   = tire_world_position_;
        float3 axis_z = (tire[0] - tire[2]) + (tire[1] - tire[3]);
        float3 axis_x = (tire[1] - tire[0]) + (tire[3] - tire[2]);

        float3 axis_y = cross(axis_z, axis_x);   //	左手座標系のためかける順序を反転
        axis_x        = cross(axis_y, axis_z);   //	再計算して直交ベクトルにする

        axis_x = normalize(axis_x);
        axis_y = normalize(axis_y);
        axis_z = normalize(axis_z);

        //	新しいボディ位置を計算(4個のタイヤの中心位置)
        float3 body_position = (tire[0] + tire[1] + tire[2] + tire[3]) * 0.25f;

        //----------------------------------------------
        //	ワールド行列を作成
        //----------------------------------------------
        mat_world_._11_12_13_14 = float4(axis_x, 0.0f);
        mat_world_._21_22_23_24 = float4(axis_y, 0.0f);
        mat_world_._31_32_33_34 = float4(axis_z, 0.0f);
        mat_world_._41_42_43_44 = float4(body_position, 1.0f);   //	ここは1.0f気を付ける！
    }
    //-------------------------------------------------------
    // モデルのワールド行列を設定
    //-------------------------------------------------------
    //	ボディの位置
    //	親の行列になるボディはスケールを持たない行列にする
    //	(スケールの二度掛けを防止する)
    matrix body_world = mat_world_;

    //	位置調整
    body_world = mul(body_world, matrix::translate(float3(0.0f, 0.2f, 0.0f)));

    {
        //	見た目をスケールする
        matrix mat_world = mul(matrix::scale(0.3f), body_world);
        model_body_->setWorldMatrix(mat_world);
    }

    //	タイヤの位置
    for(s32 i = 0; i < 4; i++) {
        matrix mat_world = matrix::scale(0.3f);   //	見た目をスケールする

        //	左右の方向で左側を反転する
        if((i & 1) == 0)                                                      //	左側のみ(偶数)
            mat_world = mul(mat_world, matrix::rotateY(180.0f * DegToRad));   //	左右の方向

        //	タイヤの回転
        mat_world = mul(mat_world, matrix::rotateX(tire_rotate_));

        //	前輪の操作
        if((i >> 1) == 0)   //	前輪のみ
            mat_world = mul(mat_world, matrix::rotateY(steering_ * 35.0f * DegToRad));

        //	位置
        mat_world = mul(mat_world, matrix::translate(tirePosition[i]));

        //	ボディに装備する(右側から親の行列をかける)
        mat_world = mul(mat_world, body_world);

        model_tire_[i]->setWorldMatrix(mat_world);
    }

    //----------------------------------------------
    // カメラの設定
    //----------------------------------------------
    VECTOR eye     = {15.0f, 10.0f, -15.0f};   //  目の位置
    VECTOR look_at = {0.0f, 5.0f, 0.0f};       //  注目点/注視点

    SetCameraPositionAndTarget_UpVecY(eye, look_at);
    SetupCamera_Perspective(60.0f * DX_PI_F / 180.0f);
}

//---------------------------------------------------------------------------
//! 描画
//---------------------------------------------------------------------------
void SceneVehicle::Draw()
{
    //	ボディ描画
    model_body_->render();

    //	タイヤホイール描画
    for(auto& x : model_tire_) {
        x->render();
    }

    //---------------------------------------------
    //	デバッグ描画
    //---------------------------------------------
    //	コンパイルはされる
    if constexpr(false) {
        for(u32 i = 0; i < 4; i++) {
            DrawSphere3D(cast(tire_world_position_[i]), 0.5f, 8, GetColor(0, 255, 0), 0, false);
        }
    }

    //---------------------------------------------
    //	デバッグUI描画
    //---------------------------------------------
    //	ブレーキペダル
    {
        constexpr s32 x = 1024;
        constexpr s32 y = 128;
        constexpr s32 w = 16;
        constexpr s32 h = 256;

        s32 height = static_cast<s32>(h * brake_pedal_);

        DrawBox(x, y, x + w, y + height, GetColor(0, 0, 255), true);
        DrawBox(x, y, x + w, y + h, GetColor(255, 255, 255), false);
    }
    //	アクセルペダル
    {
        constexpr s32 x = 1024 + 32;
        constexpr s32 y = 128;
        constexpr s32 w = 16;
        constexpr s32 h = 256;

        s32 height = static_cast<s32>(h * gas_pedal_);

        DrawBox(x, y, x + w, y + height, GetColor(0, 255, 0), true);
        DrawBox(x, y, x + w, y + h, GetColor(255, 255, 255), false);
    }
    //	ステアリングホイール
    {
        constexpr s32 w = 256;
        constexpr s32 h = 32;
        constexpr s32 x = (WINDOW_W - w) / 2;   //	中央にセンタリング
        constexpr s32 y = 640;

        //	中心位置からの相対位置
        s32 center   = WINDOW_W / 2;   //	中心位置
        s32 position = center + static_cast<s32>(w * 0.5f * steering_);

        DrawBox(position - 8, y, position + 8, y + h, GetColor(255, 255, 0), true);
        DrawBox(x, y, x + w, y + h, GetColor(255, 255, 255), false);
    }
}

//---------------------------------------------------------------------------
//! 終了
//---------------------------------------------------------------------------
void SceneVehicle::Exit()
{
}

//---------------------------------------------------------------------------
//! GUI表示
//---------------------------------------------------------------------------
void SceneVehicle::GUI()
{
}
