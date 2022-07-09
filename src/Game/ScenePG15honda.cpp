//---------------------------------------------------------------------------
//! @file   ScenePG15honda.cpp
//! @brief  サンプルシーン
//---------------------------------------------------------------------------
#include "ScenePG15honda.h"
//#include "Game/GameMain.h"
#include "System/SystemMain.h"

BP_CLASS_IMPL(ScenePG15honda, u8"PG15本田")

//	HPクラス
class HP
{
public:
    //	メンバ関数
    //	初期化
    void Init()
    {
        HP_           = HP_MAX_;
        draw_HP_      = HP_MAX_;
        is_damage_    = false;
        damage_fream_ = FREAM_;

        x_     = WINDOW_W - 130;
        y_     = WINDOW_H - 75;
        r_     = COLOR_MAX_;
        g_     = COLOR_MAX_;
        b_     = COLOR_MAX_;
        alpha_ = 255;
    }

    //	更新処理
    void Update()
    {
        //	ダメージ中だったらダメージフレームを減らす
        if(is_damage_ == true)
            damage_fream_--;
        if(damage_fream_ == 0) {
            is_damage_    = false;
            damage_fream_ = FREAM_;
        }

        //	ダメージを受けている間
        if(is_damage_) {
            //	draw_HP_を減らしていく
            draw_HP_ -= 0.3f;
            if(draw_HP_ <= HP_)
                draw_HP_ = HP_;
        }
    }

    //	描画関数
    void Draw()
    {
        //------------------------------------
        //	縦バーでのHPバー
        //------------------------------------
        //	中身
        //	赤
        DrawBox(WINDOW_W - 100,
                (WINDOW_H / 2 + 310) - ((int)draw_HP_ * 3),
                WINDOW_W - 50,
                WINDOW_H - 50,
                GetColor(255, 0, 0),
                TRUE);
        //	青
        DrawBox(WINDOW_W - 100,
                (WINDOW_H / 2 + 310) - ((int)HP_ * 3),
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
        if(HP_ == HP_MAX_) {
            r_ = 0;
            g_ = COLOR_MAX_;
            b_ = 0;
        }
        //	途中
        else if(HP_ <= 60.0f && HP_ > 30.0f) {
            r_ = COLOR_MAX_;
            g_ = 128;
            b_ = 0;
        }
        //	30％以下
        else if(HP_ <= 30.0f) {
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
            if(damage_fream_ % 5 == 0) {
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
        SetFontSize(32);
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha_);
        DrawFormatString(x_ + 3, y_ + 3, GetColor(0, 0, 0), "%3d%%", (int)HP_);
        DrawFormatString(x_, y_, GetColor(r_, g_, b_), "%3d%%", (int)HP_);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
        SetFontSize(DEFAULT_FONT_SIZE);
    }

    //---------------------------------------------------
    //	ダメージ系の関数
    //---------------------------------------------------
    //	ダメージを受ける
    //void Damage()
    //{
    //	SetIsDamage(true);
    //	HP_ -= 10.0f;
    //	if(HP_ <= 0.0f) HP_ = 0.0f;
    //}
    //	全回復
    void HpReset()
    {
        HP_      = HP_MAX_;
        draw_HP_ = HP_MAX_;
    }

    //-------------------------------------
    //	アクセッサ
    //-------------------------------------
    //	ゲッター
    bool GetIsDamage() const { return is_damage_; }
    //	セッター
    void SetIsDamage(bool flag)
    {
        is_damage_ = flag;
        if(HP_ == 0)
            is_damage_ = false;
    }

    //-------------------------------------
    //	演算子のオーバーロード
    //-------------------------------------
    //	HPを減らす(-=)
    void operator-=(float damage)
    {
        SetIsDamage(true);
        HP_ -= damage;
        //	0以下にはならない
        if(HP_ <= 0.0f)
            HP_ = 0.0f;
    }
    //	HPを増やす(+=)
    void operator+=(float heel)
    {
        HP_ += heel;
        draw_HP_ += heel;
        //	MAXより大きくはならない
        if(HP_ >= HP_MAX_)
            HP_ = HP_MAX_;
        if(draw_HP_ >= HP_MAX_)
            draw_HP_ = HP_MAX_;
    }

private:
    //	メンバ変数
    float HP_;             //	HPの値用変数
    float draw_HP_;        //	減っているのがわかりやすいように後から減らすHP
    bool  is_damage_;      //	ダメージを受けているかどうか
    int   damage_fream_;   //	ダメージを受けている時間
    //	文字列座標
    int x_;
    int y_;
    //	色変更用の変数
    int r_;
    int g_;
    int b_;
    int alpha_;   //	透過用

    //	定数
    const float HP_MAX_    = 100.0f;
    const int   COLOR_MAX_ = 255;
    const int   FREAM_     = 60;   //	1秒
};

namespace
{
class Score
{
public:
    //	読み込み関数
    void Load()
    {
        //	フォントデータの追加読み込み
        LPCSTR font_path  = "data/UI/font/GAGAGAGA-FREE.otf";   //	フォントを追加する用の関数
        LPCSTR font_path2 = "data/UI/font/NotoSerifJP-Black.otf";

        AddFontResourceEx(font_path, FR_PRIVATE, NULL);
        AddFontResourceEx(font_path2, FR_PRIVATE, NULL);
    }

    //	初期化処理
    void Init()
    {
        font_handle  = CreateFontToHandle("GAGAGAGA FREE", 40, 3, DX_FONTTYPE_ANTIALIASING_EDGE);
        font_handle2 = CreateFontToHandle("Noto Serif JP Black", 40, 3, DX_FONTTYPE_ANTIALIASING_EDGE);

        draw_score   = 0;
        score        = 0;
        fream_time   = 0;
        add_score    = 10;
        y            = 185;
        is_add_score = false;
        alpha        = 255;
    }

    //	更新処理
    void Update()
    {
        //	スコアを加算していたら
        if(is_add_score) {
            fream_time++;
            y--;
            alpha -= 10;
        }
        //	していなかったら
        else {
            y     = 185;
            alpha = 255;
        }

        if(fream_time == 3) {
            draw_score++;
            fream_time = 0;
        }

        if(draw_score >= score) {
            draw_score   = score;
            is_add_score = false;
        }
    }

    //	描画処理
    void Draw()
    {
        int white    = GetColor(255, 255, 255);
        int sky_blue = GetColor(0, 255, 255);

        DrawFormatStringToHandle(50, 220, white, font_handle2, "%05d", draw_score);
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

    //	スコア加算
    //void AddScore()
    //{
    //	score += 10.0f;
    //}

    //	スコアリセット
    void Reset() { score = 0; }

    //-------------------------------------
    //	アクセッサ
    //-------------------------------------
    bool GetIsAddScore() { return is_add_score; }

    //-------------------------------------
    //	演算子のオーバーロード
    //-------------------------------------
    //	スコアを増やす(+=)
    void operator+=(int score_num)
    {
        is_add_score = true;
        score += score_num;
    }

private:
    //	変数
    const char* num_text     = "0123456";   //	文字列
    int         font_handle  = -1;          //	フォントハンドル
    int         font_handle2 = -1;          //	フォントハンドル2
    bool        is_add_score;
    int         fream_time;
    int         alpha;
    //	スコア
    int score;
    int draw_score;
    int add_score;
    int y;   //	スコアのx座標
};

//	クラス定義
static HP    hp_;
static Score score_;
}   // namespace
//---------------------------------------------------------------------------
//! 初期化
//---------------------------------------------------------------------------
bool ScenePG15honda::Init()
{
    ShowGrid(false);

    //	作ったフォントを読み込む
    //font_handle = LoadFontDataToHandle("data/UI/Font/GAGAGAGA.dft");

    hp_.Init();
    score_.Load();
    score_.Init();

    return true;
}

//---------------------------------------------------------------------------
//! 更新
//! @param  [in]    delta   経過時間
//---------------------------------------------------------------------------
void ScenePG15honda::Update([[maybe_unused]] float delta)
{
    hp_.Update();
    score_.Update();

    //	ダメージを受けていたら (is_damage_ = true)
    if(hp_.GetIsDamage())
        return;
    //	Sキーでダメージ
    if(IsKeyOn(KEY_INPUT_S)) {
        //	hp_.Damage();
        hp_.operator-=(10.0f);
    }

    //	Dキーで回復
    if(IsKeyOn(KEY_INPUT_D)) {
        hp_.operator+=(5.0f);
    }

    //	スコア加算中なら
    if(score_.GetIsAddScore())
        return;
    //	Lキーでスコア加算
    if(IsKeyOn(KEY_INPUT_L)) {
        score_.operator+=(10);
    }

    //	SPACEキーでリセット
    if(IsKeyOn(KEY_INPUT_SPACE)) {
        hp_.HpReset();
        score_.Reset();
    }
}

//---------------------------------------------------------------------------
//! 描画
//---------------------------------------------------------------------------
void ScenePG15honda::Draw()
{
    //	描画
    //DrawStringToHandle(50, 150, num_text, GetColor(0, 0, 0), font_handle);
    //DrawStringToHandle(50, 150, num_text, GetColor(255, 255, 255), font_handle);

    hp_.Draw();
    score_.Draw();
}

//---------------------------------------------------------------------------
//! 終了
//---------------------------------------------------------------------------
void ScenePG15honda::Exit()
{
    ShowGrid(true);
    //DeleteFontToHandle(font_handle);
    score_.Exit();
}

//---------------------------------------------------------------------------
//! GUI表示
//---------------------------------------------------------------------------
void ScenePG15honda::GUI()
{
}
