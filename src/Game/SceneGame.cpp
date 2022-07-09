#include "SceneGame.h"
#include "Const.h"
//===========システム系===========
#include <System/Component/ComponentModel.h>
#include <System/Component/ComponentCollisionModel.h>
#include <System/Component/ComponentCollisionSphere.h>
#include <System/Component/ComponentCollisionCapsule.h>
#include <System/Component/ComponentCamera.h>

#include <cmath>

BP_CLASS_IMPL(SceneGame, u8"自分のゲーム");

namespace
{
static float player_HP_   = HP_MAX_;   //	プレイヤーの体力
static int   enemy_num_   = ENEMY_MAX_ - 5;
static int   enemy_frame_ = 0;

static int enemy_kill_num_ = 0;   //	enemyを倒した数
static int clear_item_num_ = 0;   //	クリアに必要なアイテムの数を数える変数

static bool stop_flag        = false;   //	時間が止まっているか否か
static bool game_over_flag_  = false;   //	今がゲームオーバーしたかどうかの変数
static bool game_clear_flag_ = false;   //	今がゲームクリアしたかどうかの変数

static int game_frame_;       //	ゲームのフレーム数用
static int game_look_time_;   //	見える時間(フレームじゃなくて秒でだしたい用)

//------------------------------------------------------------------
//	プレイヤークラス
//------------------------------------------------------------------
class Player : public Object
{
public:
    //------------------------------------
    // 初期化処理
    //------------------------------------
    bool Init() override
    {
        __super::Init();

        //---------------------------------------------------
        // モデルコンポーネント(0.08倍)
        //---------------------------------------------------
        auto player_ = AddComponent<ComponentModel>("data/Game/Player/model.mv1")
                           ->SetScaleAxisXYZ({
                               0.08f
        })
                           //	アニメーション{アニメーション名,ファイル名,ファイル内のアニメーション番号,再生速度}
                           ->SetAnimation({
                               {"idle", "data/Game/Player/Anim/Idle.mv1", 0, 1.0f},
                               {"run", "data/Game/Player/Anim/Run.mv1", 0, 1.0f},
                               {"attack", "data/Game/Player/Anim/Sword Attack.mv1", 0, 1.0f},
                               {"die", "data/Game/Player/Anim/Die.mv1", 0, 1.0f},
                           });

        // コリジョン(当たり判定)
        //	自分
        auto col = AddComponent<ComponentCollisionCapsule>();
        col->SetTranslate({0, 0, 0})->SetRadius(1.5f)->SetHeight(12)->UseGravity();
        col->SetCollisionGroup(ComponentCollision::CollisionGroup::PLAYER);

        // カメラ
        auto camera = AddComponent<ComponentCamera>()->SetPositionAndTarget({0, 35, 60}, {0, 20, 0});

        //-------------------------------------------------
        //	武器
        //-------------------------------------------------

        //	アタッチ用の武器
        model_sword_ = std::make_shared<Model>("data/Game/Player/Sword.mv1");

        auto col2 = AddComponent<ComponentCollisionCapsule>();
        col2->SetRotationAxisXYZ({0, 0, -80})->SetTranslate({10, 2, 0})->SetRadius(0.2f)->SetHeight(7);
        col2->SetHitCollisionGroup((u32)ComponentCollision::CollisionGroup::ENEMY);
        col2->AttachToModel(36);   //	右手

        //	Player関係変数の初期化
        speed_        = 1.0f;
        buffer_speed_ = 0.0f;
        rot_y_        = 0.0f;
        is_attack_    = false;   //	アタック中かどうか

        item_frame_flag_ = false;
        item_frame_      = ITEM_FRAME_;
        stop_time_       = STOP_TIME_;   //	止めることのできるフレーム数(今は180fなので三秒)
        time_stop_flag_  = true;         //	true:止めれる false:止めれない
        stop_flag_ct_    = COOL_TIME_;   //	次止められるまでのクールタイム(今は120fなので二秒)

        //	HP関係変数の初期化
        draw_HP_      = HP_MAX_;
        is_damage_    = false;
        damage_frame_ = FRAME_;

        x_     = WINDOW_W - 130;
        y_     = WINDOW_H - 75;
        r_     = COLOR_MAX_;
        g_     = COLOR_MAX_;
        b_     = COLOR_MAX_;
        alpha_ = 255;

        return true;
    }

    //------------------------------------
    // 更新処理
    //------------------------------------
    void Update([[maybe_unused]] float delta) override
    {
        //----------------------------------------
        // 武器の位置
        //----------------------------------------
        //	プレイヤーを取得
        auto player_model_ = GetComponent<ComponentModel>()->GetModel();

        //	武器をアタッチするフレームの番号を検索
        auto attach_frame_ = MV1SearchFrame(player_model_, "mixamorig:RightHand");
        //	武器をアタッチするフレームのワールド行列を取得
        auto mat_weapon_frame_ = MV1GetFrameLocalWorldMatrix(player_model_, attach_frame_);

        mat_world = matrix::scale(0.055f);

        mat_world = mul(mat_world, matrix::rotateZ(-80.0f * DegToRad));              //	方向調整
        mat_world = mul(mat_world, matrix::translate(float3(-10.0f, 5.0f, 0.0f)));   //	位置調整

        // 手に装備 (装備したい関節フレームの行列を後ろから掛ける)
        mat_world = mul(mat_world, cast(mat_weapon_frame_));

        model_sword_->setWorldMatrix(mat_world);

        //----------------------------------------
        // ダメージを受けている時間
        //----------------------------------------
        if(is_damage_ == true)
            damage_frame_--;

        if(damage_frame_ == 0) {
            damage_frame_ = FRAME_;
            is_damage_    = false;
        }

        //---------------------------------------
        // HPの更新処理
        //---------------------------------------
        //	ダメージ中だったらダメージフレームを減らす
        if(is_damage_ == true)
            damage_frame_--;
        if(damage_frame_ == 0) {
            is_damage_    = false;
            damage_frame_ = FRAME_;
        }

        //	ダメージを受けている間
        if(is_damage_) {
            //	draw_HP_を減らしていく
            draw_HP_ -= 0.5f;
            if(draw_HP_ <= player_HP_)
                draw_HP_ = player_HP_;
        }

        //----------------------------------------------
        // プレイヤーの動き
        //----------------------------------------------
        auto mdl = GetComponent<ComponentModel>();

        float3 move = float3(0, 0, 0);

        //	カメラ操作(左右)
        if(IsKeyRepeat(KEY_INPUT_A)) {
            rot_y_ += -2.0f;
            SetRotationAxisXYZ({0, rot_y_, 0});
        }
        if(IsKeyRepeat(KEY_INPUT_D)) {
            rot_y_ += 2.0f;
            SetRotationAxisXYZ({0, rot_y_, 0});
        }

        //printfDx("%s", std::string(mdl->GetPlayAnimationName()).c_str());

        //	gameover_flag_がtrueだったら
        if(game_over_flag_ == true) {
            //if(mdl->GetPlayAnimationName() != "die") mdl->PlayAnimation("die", false, 0.2f);
            if(std::string(mdl->GetPlayAnimationName()).c_str() != "die")
                mdl->PlayAnimation("die", false, 0.2f);
            //	これより下の作業はしない
            return;
        }

        //	プレイヤー(モデル)移動
        if(IsKeyRepeat(KEY_INPUT_UP)) {
            float3 vec = GetMatrix().axisZ();
            move += -vec;
        }
        if(IsKeyRepeat(KEY_INPUT_RIGHT)) {
            float3 vec = GetMatrix().axisX();
            move += -vec;
        }
        if(IsKeyRepeat(KEY_INPUT_DOWN)) {
            float3 vec = GetMatrix().axisZ();
            move += vec;
        }
        if(IsKeyRepeat(KEY_INPUT_LEFT)) {
            float3 vec = GetMatrix().axisX();
            move += vec;
        }

        //	スペースキーを押したら攻撃(モーション)
        if(IsKeyOn(KEY_INPUT_SPACE)) {
            if(mdl->GetPlayAnimationName() != "attack")
                mdl->PlayAnimation("attack", false, 0.3f);
            is_attack_ = true;
        }

        if(is_attack_ == false) {
            if(length(move).x > 0) {
                //	動いてる
                move = normalize(move);

                //	動いた方向へ向きを変えている
                float x     = -move.x;
                float z     = -move.z;
                float theta = atan2(x, z) * RadToDeg - rot_y_;
                mdl->SetRotationAxisXYZ({0, theta, 0});

                if(mdl->GetPlayAnimationName() != "run")
                    mdl->PlayAnimation("run", true, 0.2f);
            }
            else {
                //	止まってる
                if(mdl->GetPlayAnimationName() != "idle")
                    mdl->PlayAnimation("idle", true, 0.2f);
            }
            move *= speed_ * (delta * 60.0f);

            // 地面移動スピードを決定する
            AddTranslate(move);
        }
        else {
            if(mdl->IsPlaying() == false)
                is_attack_ = false;
        }

        //	アイテムを取ると速度が上がる
        if(item_frame_flag_ == true) {
            item_frame_--;
            if(item_frame_ == 0) {
                item_frame_      = ITEM_FRAME_;
                item_frame_flag_ = false;
                SetSpeed(buffer_speed_);
            }
        }

        //--------------------------------------
        // 時間を止める実装の部分
        //--------------------------------------
        //	Sキーを押したら
        if(IsKeyOn(KEY_INPUT_S)) {
            //	時間が止められるなら(trueなら)止める
            if(time_stop_flag_)
                stop_flag = true;
        }
        //	時間が止まっていたら停止時間を減らす
        if(stop_flag == true)
            stop_time_--;
        if(stop_time_ <= 0) {
            stop_flag       = false;
            stop_time_      = STOP_TIME_;
            time_stop_flag_ = false;
        }

        //	クールタイム
        if(time_stop_flag_ == false)
            stop_flag_ct_--;
        if(stop_flag_ct_ == 0) {
            time_stop_flag_ = true;
            stop_flag_ct_   = COOL_TIME_;
        }
    }

    //------------------------------------
    //	描画関数
    //------------------------------------
    void LateDraw() override
    {
        //------------------------------------
        // 武器の描画
        //------------------------------------
        model_sword_->render();

        //------------------------------------
        //	HPバー
        //------------------------------------
        //	中身
        //	見やすいように裏に灰色をおく
        DrawBox(WINDOW_W - 100, WINDOW_H / 2 + 10, WINDOW_W - 50, WINDOW_H - 50, GetColor(128, 128, 128), TRUE);
        //	赤
        DrawBox(WINDOW_W - 100,
                (WINDOW_H / 2 + 310) - ((int)draw_HP_ * 3),
                WINDOW_W - 50,
                WINDOW_H - 50,
                GetColor(255, 0, 0),
                TRUE);
        //	青
        DrawBox(WINDOW_W - 100,
                (WINDOW_H / 2 + 310) - ((int)player_HP_ * 3),
                WINDOW_W - 50,
                WINDOW_H - 50,
                GetColor(0, 128, 255),
                TRUE);
        //	外
        DrawBox(WINDOW_W - 100, WINDOW_H / 2 + 10, WINDOW_W - 50, WINDOW_H - 50, GetColor(255, 255, 255), FALSE);

        //------------------------------------
        //	体力で文字の色を変える
        //------------------------------------
        //	満タン
        if(player_HP_ == HP_MAX_) {
            r_ = 0;
            g_ = COLOR_MAX_;
            b_ = 0;
        }
        //	途中
        else if(player_HP_ <= 60.0f && player_HP_ > 30.0f) {
            r_ = COLOR_MAX_;
            g_ = 128;
            b_ = 0;
        }
        //	30％以下
        else if(player_HP_ <= 30.0f) {
            r_ = COLOR_MAX_;
            g_ = 0;
            b_ = 0;
        }
        //	元の色
        else {
            r_ = COLOR_MAX_;
            g_ = COLOR_MAX_;
            b_ = COLOR_MAX_;
        }

        //	ダメージを受けている間は文字の透明度、色を変える
        if(is_damage_) {
            //	透明度
            if(damage_frame_ % 5 == 0) {
                alpha_ = 0;
            }
            else {
                alpha_ = 255;
            }

            //	色
            r_ = COLOR_MAX_;
            g_ = 0;
            b_ = 0;
        }

        //------------------------------------
        //	文字
        //------------------------------------
        //	HPバー
        SetFontSize(32);
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha_);
        DrawFormatString(x_ + 3, y_ + 3, GetColor(0, 0, 0), "%3d%%", (int)player_HP_);
        DrawFormatString(x_, y_, GetColor(r_, g_, b_), "%3d%%", (int)player_HP_);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
        SetFontSize(DEFAULT_FONT_SIZE);
    }

    //------------------------------------------
    // GUI
    //------------------------------------------
    void GUI() override
    {
        __super::GUI();

        //	GUI描画
        ImGui::Begin(GetName().data());
        {
            ImGui::Separator();

            ImGui::DragFloat(u8"速度", &speed_, 0.01f, 0.01f, 10.0f, "%2.2f");
            //	デバック用HP描画
            ImGui::DragFloat("HP", &player_HP_, 0.0f, 0.01f, 100.0f, "%3.2f");
        }
        ImGui::End();
    }

    //------------------------------------------
    //	コリジョンの当たり判定
    //------------------------------------------
    void OnHit([[maybe_unused]] const ComponentCollision::HitInfo& hitInfo) override
    {
        // 次のownerのオブジェクトと当たった!
        auto owner = hitInfo.hit_collision_->GetOwnerPtr();

        //	地面以外に当たったら
        if(owner->GetName() != "Ground" && owner->GetNameDefault() != "enemy") {
            Scene::ReleaseObject(owner);

            //	剣が敵に当たって倒したら
            //	(今はアイテム以外に当たったらと同じ)
            if(owner->GetName() != "item" && owner->GetName() != "clear_item") {
                enemy_kill_num_++;
            }
        }

        //	エネミーに当たったらHPを減らす
        if(owner->GetNameDefault() == "enemy") {
            if(is_damage_ == false) {
                PlayerDamege();
                is_damage_ = true;
                game_look_time_ += GAMETIME_ADD_;   //	倒したら時間を増やすようにしたいにしたい
            }
        }

        //	アイテムと当たったら
        //	速度が上がるアイテム
        if(owner->GetName() == "item") {
            buffer_speed_ = speed_;
            SetSpeed(2.0f);
            item_frame_flag_ = true;
        }
        //	クリア用アイテム
        if(owner->GetName() == "clear_item") {
            clear_item_num_++;
        }

        // 当たりで移動させる(これが無ければめり込む)
        __super::OnHit(hitInfo);
    }

    //	ダメージ関数
    float PlayerDamege()
    {
        player_HP_ -= 10.0f;

        if(player_HP_ <= 0.0f) {
            player_HP_ = 0.0f;
        }

        return player_HP_;
    }

    //-----------------------------------------------------
    //	アクセッサ
    //-----------------------------------------------------
    //==スピード==
    void SetSpeed(float s) { speed_ = s; }

    float GetSpeed() const { return speed_; }
    //	止まっている時間を取得
    int GetStopTime() const { return stop_time_; }

private:
    //	モデル系のメンバ変数定義
    std::shared_ptr<Model> model_sword_;   //	!< 3Dモデル
    matrix                 mat_world;      //	ワールド座標

    //	メンバ変数の定義
    //	Player
    float speed_;
    float buffer_speed_;
    float rot_y_;
    bool  is_attack_;   //	アタック中かどうか

    bool item_frame_flag_;   //	アイテムを使っているかフラグ
    int  item_frame_;        //	アイテム使用中のフレーム数
    int  stop_time_;         //	止めることのできるフレーム数(今は180fなので三秒)
    bool time_stop_flag_;    //	true:止めれる false:止めれない
    int  stop_flag_ct_;      //	次止められるまでのクールタイム

    //	HP
    float draw_HP_;        //	減っているのがわかりやすいように後から減らすHP
    bool  is_damage_;      //	ダメージを受けているかどうか
    int   damage_frame_;   //	ダメージを受けている時間
    //	文字列座標
    int x_;
    int y_;
    //	色変更用の変数
    int r_;
    int g_;
    int b_;
    int alpha_;   //	透過用
};

//------------------------------------------------------------------
//	エネミークラス
//------------------------------------------------------------------
class Enemy : public Object
{
public:
    bool Init() override
    {
        __super::Init();

        // モデルコンポーネント
        auto enemy_model_ = AddComponent<ComponentModel>("data/Game/Enemy/model.mv1")
                                ->SetScaleAxisXYZ({
                                    0.1f
        })
                                ->SetAnimation({
                                    {"idle", "data/Game/Enemy/Anim/Idle.mv1", 0, 1.0f},
                                    {"walk", "data/Game/Enemy/Anim/Walk.mv1", 0, 1.0f},
                                    {"attack", "data/Game/Enemy/Anim/Attack.mv1", 0, 1.0f},
                                });

        //	攻撃する方の手
        auto hand_col = AddComponent<ComponentCollisionSphere>();
        hand_col->SetRadius(1.5f)->AttachToModel(enemy_model_->GetNodeIndex("mixamorig:RightHand"));
        hand_col->SetCollisionGroup(ComponentCollision::CollisionGroup::ENEMY);
        hand_col->SetHitCollisionGroup((u32)ComponentCollision::CollisionGroup::PLAYER |
                                       (u32)ComponentCollision::CollisionGroup::ETC);

        // コリジョン(当たり判定)
        auto col = AddComponent<ComponentCollisionCapsule>();
        col->SetTranslate({0, 0, 0})->SetRadius(3.0)->SetHeight(15);
        col->SetCollisionGroup(ComponentCollision::CollisionGroup::ENEMY);
        col->UseGravity();

        return true;
    }

    void Update([[maybe_unused]] float delta) override
    {
        //---------------------------------------------------------
        // Enemyの基本の動き
        //---------------------------------------------------------
        auto mdl = GetComponent<ComponentModel>();

        //	ゲームが終了していたら
        if(game_clear_flag_ || game_over_flag_) {
            //	モーションをIdleにして、それ以外のことはしない
            if(mdl->GetPlayAnimationName() != "walk")
                mdl->PlayAnimation("walk", true, 0.2f);
            return;
        }

        //	追尾するためにプレイヤーをプロジェクト内から探して持ってくる
        auto player = Scene::GetObjectPtr<Player>("Player");

        auto target = player->GetTranslate();   //	target：相手
        auto pos    = GetTranslate();           //	pos   ：自分

        //	時間が止まっていたら(true)下の処理は行わない
        if(stop_flag)
            return;
        //	自分からターゲットへの距離のベクトル
        float3 move = (target - pos);

        //	playerが近い時にアタックがしたいのでアタックフラグ
        bool is_attack = false;
        //	ある程度playerと近いと攻撃モーションをしてくる
        if(move.x < 0.5f || move.z < 0.5f) {
            is_attack = true;
            if(mdl->GetPlayAnimationName() != "attack")
                mdl->PlayAnimation("attack", true, 0.2f);
        }
        else {
            is_attack = false;
        }

        if(!is_attack) {
            //	プレイヤーに向かって歩く
            if(length(move).x > 0) {
                //	動いてる
                move = normalize(move);

                //	動いた方向へ向きを変えている
                float x     = -move.x;
                float z     = -move.z;
                float theta = atan2(x, z) * RadToDeg - rot_y_;
                mdl->SetRotationAxisXYZ({0, theta, 0});

                if(mdl->GetPlayAnimationName() != "walk")
                    mdl->PlayAnimation("walk", true, 0.2f);
            }
            else {
                //	止まってる
                if(mdl->GetPlayAnimationName() != "idle")
                    mdl->PlayAnimation("idle", true, 0.2f);
            }

            move *= speed_ * (delta * 60.0f);

            // 地面移動スピードを決定する
            AddTranslate(move);
        }
        else {
            if(mdl->IsPlaying() == false)
                is_attack = false;
        }
    }

    //-------------------------------------------------
    // コリジョンの当たり判定
    //-------------------------------------------------
    void OnHit([[maybe_unused]] const ComponentCollision::HitInfo& hitInfo) override
    {
        // 当たりで移動させる(これが無ければめり込みます)
        __super::OnHit(hitInfo);
    }

    //-----------------------------------------------------
    //	アクセッサ
    //-----------------------------------------------------
    //	==スピード==
    void SetSpeed(float s) { speed_ = s; }

    float GetSpeed() const { return speed_; }

private:
    float speed_ = 0.3f;
    float rot_y_ = 0.0f;
};

//------------------------------------------------
//	アイテムクラス(今はスピードUPアイテムだけ)
//------------------------------------------------
class Item : public Object
{
public:
    bool Init() override
    {
        __super::Init();

        //	オブジェクトを作る
        AddComponent<ComponentCollisionSphere>()->SetTranslate({0, 0, 0})->SetRadius(1.5)->SetHitCollisionGroup(
            (u32)ComponentCollision::CollisionGroup::PLAYER);
        SetScaleAxisXYZ({0.4f})->SetTranslate({50, 10, 10});

        return true;
    }

    void Draw() override
    {
        //	SetUseLighting(false);	  //  ライティングOFF
        DrawSphere3D(VGet(50, 10, 10), 1.5f, 16, GetColor(0, 128, 255), 0, true);
        //	SetUseLighting(true);
    }
};

//------------------------------------------------
//	テキストクラス
//------------------------------------------------
class Text : public Object
{
public:
    //	初期化関数
    bool Init() override
    {
        __super::Init();

        //	フォントデータの追加読み込み
        LPCSTR font_path = "data/UI/font/NotoSerifJP-Black.otf";   //	フォントを追加する用の関数

        AddFontResourceEx(font_path, FR_PRIVATE, NULL);

        font_handle2 = CreateFontToHandle("Noto Serif JP Black", 40, 3, DX_FONTTYPE_ANTIALIASING_EDGE);

        draw_score   = 0;
        score        = 0;
        frame_time   = 0;
        add_score    = 10;
        y            = 185;
        is_add_score = false;
        alpha        = 255;

        return true;
    }

    //	更新処理
    void Update()
    {
        //	スコアを加算していたら
        if(is_add_score) {
            frame_time++;
            y--;
            alpha -= 10;
        }
        //	していなかったら
        else {
            y     = 185;
            alpha = 255;
        }

        if(frame_time == 3) {
            draw_score++;
            frame_time = 0;
        }

        if(draw_score >= score) {
            draw_score   = score;
            is_add_score = false;
        }
    }

    //	描画関数
    void LateDraw() override
    {
        //	時間停止中がわかりやすいように設定
        if(stop_flag == true) {
            SetDrawBlendMode(DX_BLENDMODE_ALPHA, 128);
            DrawBox(0, 0, WINDOW_W, WINDOW_H, GetColor(50, 50, 50), true);
            SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
            //	停止しているときの時間(playerクラスの中に入っているアクセッサで取得したい)
            auto time_ = Scene::GetObjectPtr<Player>()->GetStopTime();
            SetFontSize(32);
            DrawFormatStringToHandle(WINDOW_W - 250,
                                     WINDOW_H - 150,
                                     GetColor(255, 0, 0),
                                     font_handle2,
                                     "Time\n  %2d",
                                     time_,
                                     true);
            SetFontSize(DEFAULT_FONT_SIZE);
        }

        //-----------------------------------------------------------------------
        // ゲームクリア・ゲームオーバーの描画
        //-----------------------------------------------------------------------
        //	ゲームクリア
        if(game_clear_flag_) {
            SetFontSize(48);
            DrawString(WINDOW_W / 2 - 100, WINDOW_H / 2, "GameClear", GetColor(255, 255, 255));
            SetFontSize(DEFAULT_FONT_SIZE);
        }
        //	ゲームオーバー
        if(game_over_flag_) {
            SetFontSize(48);
            DrawString(WINDOW_W / 2 - 100, WINDOW_H / 2, "GameOver", GetColor(255, 255, 255));
            SetFontSize(DEFAULT_FONT_SIZE);
        }

        if(game_over_flag_)
            return;
        //	画面の時間表示
        SetFontSize(32);
        DrawFormatStringToHandle(120, 16, GetColor(255, 255, 255), font_handle2, "Time：%2d", game_look_time_, true);
        SetFontSize(DEFAULT_FONT_SIZE);

        //	スコア表示
        int white    = GetColor(255, 255, 255);
        int sky_blue = GetColor(0, 255, 255);

        DrawFormatStringToHandle(WINDOW_W - 200, 220, white, font_handle2, "%05d", draw_score);
        if(is_add_score) {
            SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
            DrawFormatStringToHandle(120, y, sky_blue, font_handle2, "+%2d", add_score);
            SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
        }
    }

    //	終了処理
    void Exit()
    {
        DeleteFontToHandle(font_handle);
        DeleteFontToHandle(font_handle2);
    }

    //-------------------------------------
    //	アクセッサ
    //-------------------------------------
    //	スコアが入るかどうか
    bool GetIsAddScore() { return is_add_score; }
    //	スコア加算
    void AddScore(int num)
    {
        is_add_score = true;
        score += num;
    }

private:
    //	変数
    int  font_handle  = -1;   //	フォントハンドル
    int  font_handle2 = -1;   //	フォントハンドル2
    bool is_add_score;
    int  frame_time;
    int  alpha;
    //	スコア
    int score;
    int draw_score;
    int add_score;
    int y;   //	スコアのY座標
};
}   // namespace

bool SceneGame::Init()
{
    //----------------------------------------------------------------------------------
    // ステージ 初期化
    //----------------------------------------------------------------------------------
    {
        auto obj = Scene::CreateObject<Object>()->SetName("Ground");
        obj->AddComponent<ComponentModel>("data/Sample/SwordBout/Stage/Stage00.mv1");
        obj->AddComponent<ComponentCollisionModel>()->AttachToModel();
    }

    stop_flag = false;

    //----------------------------------------------------------------------------------
    // Player 初期化(生成)
    //----------------------------------------------------------------------------------
    auto player = Scene::CreateObject<Player>();
    player->SetName("Player");
    player->SetTranslate({0, 15, 0});

    //----------------------------------------------------------------------------------
    // Enemy 初期化(生成)
    //----------------------------------------------------------------------------------
    //	複数体つくる
    //	for(int i = 0; i < ENEMY_MAX_; i++) {
    auto enemy = Scene::CreateObject<Enemy>();
    enemy->SetName("enemy");
    //		enemy->SetTranslate({GetRand(200), 0, GetRand(200)});
    enemy->SetTranslate({10, 0, 10});
    //	}

    //--------------------------------------------------------------
    //	アイテム(のちに増やす)
    //--------------------------------------------------------------
    auto item = Scene::CreateObject<Item>();
    item->SetName("item");

    game_frame_     = 0;
    game_look_time_ = 20;

    //------------------------------------------------------
    // ステージ描画
    //------------------------------------------------------
    //	ステージよりも後に描画
    auto text = Scene::CreateObject<Text>();

    return true;
}

void SceneGame::Update([[maybe_unused]] float delta)
{
    //--------------------------------------------------------
    //	プレイヤー関係
    //--------------------------------------------------------

    //--------------------------------------------------------
    // Enemy関係
    //--------------------------------------------------------
    //	時間経過で増えていく敵を実装したい
    //enemy_frame_++;
    //if(enemy_frame_ == 120) {
    //	if(enemy_num_ >= ENEMY_MAX_ + 5) return;

    //	//	数秒ごとに複数体作る
    //	auto e = Scene::CreateObject<Enemy>();
    //	e->SetName("enemy");
    //	e->SetTranslate({150, 0, 0});

    //	enemy_num_++;
    //	enemy_frame_ = 0;
    //}

    //----------------------------------------------------------------
    // アイテム系のアップデート
    //----------------------------------------------------------------

    //	敵を倒した数によってアイテムを落とす
    if(enemy_kill_num_ == 3) {
        auto item2 = Scene::CreateObject<Item>()->SetName("clear_item");
        item2->AddComponent<ComponentCollisionSphere>()->SetTranslate({0, 0, 0})->SetRadius(1.5);
        item2->SetScaleAxisXYZ({0.5f})->SetTranslate({20, 10, 20});
        enemy_kill_num_ = 0;
    }

    //-----------------------------------------------------------------
    // 時間制限
    //-----------------------------------------------------------------
    //	時間制限用(今は10秒)
    //if(stop_flag == false) {
    //	game_frame_++;
    //	if(game_frame_ == 60) {
    //		game_look_time_--;
    //		game_frame_ = 0;
    //	}
    //}

    //------------------------------------------------------------------
    // ゲームオーバー,ゲームクリア判定
    //------------------------------------------------------------------
    //	ゲームオーバー判定
    //		┗ 時間切れとPLの体力切れ
    if(game_look_time_ == 0 || player_HP_ == 0)
        game_over_flag_ = true;

    //	ゲームクリア判定
    //		┗ 敵を倒したら出るアイテムを集めきったら
    if(clear_item_num_ == 3)
        game_clear_flag_ = true;
}

void SceneGame::Draw()
{
}

void SceneGame::Exit()
{
    // タイトル終了時に行いたいことは今はない
}

void SceneGame::GUI()
{
}
