//---------------------------------------------------------------------------
//! @file   SceneWater.cpp
//! @brief  水面サンプルシーン
//---------------------------------------------------------------------------
#include "SceneWater.h"
#include <System/Graphics/Model.h>
#include <System/Component/ComponentCamera.h>

#include "System/SystemMain.h"   // ShowGrid

BP_CLASS_IMPL(SceneWater, u8"水面サンプル");

namespace
{

//===========================================================================
//! 水面クラス
//===========================================================================
class Water
{
public:
    //! コンストラクタ
    //! @param  [in]    width       配列幅
    //! @param  [in]    height      配列高さ
    //! @param  [in]    position    配置座標
    //! @param  [in]    scale       描画スケール(default:1.0f)
    Water(u32 width, u32 height, const float3& position, f32 scale = 1.0f)
        : width_(width)
        , height_(height)
        , position_(position)
        , scale_(scale)
    {
        water_height_.resize(width_ * height_);
        water_height2_.resize(width_ * height_);

        reset();

        // TEST:水面中央を高くする
        if constexpr(true) {
            water(width_ / 2, height_ / 2) = 1.0f;
        }
    }

    //! 波をリセット
    void reset()
    {
        // 高さを初期化
        for(auto& x : water_height_) {
            x = float2(0.0f, 0.0f);
        }
    }

    //! 更新
    void update([[maybe_unused]] f32 dt)
    {
        constexpr f32 attenuation = 0.994f;   // 減衰係数

        // 波動方程式 有限差分法
        // 波の高さの変化の加速度に定数を掛けたもの = 現在の波の高さに「ラプラシアンフィルタ」を掛けたものと等価
        // +---+---+---+
        // | 0 | 1 | 0 |
        // +---+---+---+
        // | 1 |-4 | 1 |
        // +---+---+---+
        // | 0 | 1 | 0 |
        // +---+---+---+

        // 二次元配列アクセスのための短縮表記
        auto w = [&](s32 x, s32 y) { return water_height_[y * width_ + x]; };

        f32 velocity2 = std::clamp(0.125f, 0.0f, 0.5f);   // 速度の2乗 [0.0f-0.5f]

        for(s32 y = 1; y < height_ - 1; ++y) {
            for(s32 x = 1; x < width_ - 1; ++x) {
                // 中心の情報 [現在の高さ, 1フレーム前の高さ]
                float2 center = w(x, y);

                f32 neighbor = w(x + 1, y).x + w(x - 1, y).x + w(x, y + 1).x + w(x, y - 1).x;

                float height = 2.0f * center.x - center.g + velocity2 * (neighbor - 4.0f * center.x);
                center.y     = center.x;
                center.x     = height;

                // 波が立たないように少しづつ減衰させる
                center.xy *= attenuation;

                water_height2_[y * width_ + x] = center;
            }
        }

        // ダブルバッファを交換
        std::swap(water_height_, water_height2_);
    }

    //! 描画
    void render()
    {
        // 二次元配列アクセスのための短縮表記
        auto w = [&](s32 x, s32 y) { return water_height_[y * width_ + x].x; };

        // 色を計算
        auto color = [](f32 y) {
            auto r = 0;
            auto g = static_cast<s32>(y * 1024.0f + 64.0f);
            auto b = static_cast<s32>(y * 256.0f + 224.0f);

            r = std::clamp(r, 0, 255);
            g = std::clamp(g, 0, 255);
            b = std::clamp(b, 0, 255);

            return GetColorU8(r, g, b, 255);
        };

        std::vector<VERTEX3D> v;

        // ライティング無効化
        SetUseLighting(false);

        // 水面ワイヤーフレームを描画
        v.resize(width_);
        for(s32 iy = 0; iy < height_; ++iy) {
            for(s32 ix = 0; ix < width_; ++ix) {
                f32 x = static_cast<f32>(ix) * scale_;
                f32 z = static_cast<f32>(iy) * scale_;

                float3 p = float3(x, w(ix, iy), z);

                v[ix].pos = cast(position_ + p);
                v[ix].dif = color(p.y);
                v[ix].spc = GetColorU8(0, 0, 0, 0);
            }
            // LineStripで描画
            DrawPrimitive3D(v.data(), width_, DX_PRIMTYPE_LINESTRIP, DX_NONE_GRAPH, false);
        }

        v.resize(height_);
        for(s32 ix = 0; ix < width_; ++ix) {
            for(s32 iy = 0; iy < height_; ++iy) {
                f32 x = static_cast<f32>(ix) * scale_;
                f32 z = static_cast<f32>(iy) * scale_;

                float3 p = float3(x, w(ix, iy), z);

                v[iy].pos = cast(position_ + p);
                v[iy].dif = color(p.y);
                v[iy].spc = GetColorU8(0, 0, 0, 0);
            }
            // LineStripで描画
            DrawPrimitive3D(v.data(), height_, DX_PRIMTYPE_LINESTRIP, DX_NONE_GRAPH, false);
        }

        // 元に戻す
        SetUseLighting(true);
    }

    //! 指定XZ座標の位置に力を加える
    //! @param  [in]    world_position  力を加える位置
    //! @param  [in]    impulse         力
    //! @retval true    正常に適用された場合
    //! @retval false   範囲外で適用出来なかった場合
    //! @note 指定座標が水面よりも高い位置にある場合や範囲外の場合は適用されません
    bool addImpulse(const float3& p, f32 impulse)
    {
        // 水面よりも高い位置なら終了
        if(position_.y < static_cast<f32>(p.y)) {
            return false;
        }

        // 相対座標
        float3 relative = p - position_;
        int3   index    = relative / scale_ + 0.5f;   // 四捨五入

        // 水面の範囲外なら終了
        if(any(index.xz < 1)) {
            return false;
        }
        if(width_ - 1 <= index.x) {
            return false;
        }
        if(height_ - 1 <= index.z) {
            return false;
        }

        // 力を加える
        water(index.x, index.z) += impulse;

        return true;
    }

    //! 水面の高さを参照
    f32& water(s32 x, s32 y) { return water_height_[y * width_ + x].f32[0]; };

    //! 配列幅を取得
    s32 width() const { return width_; }

    //! 配列たかさを取得
    s32 height() const { return height_; }

    //! 位置を取得
    float3 position() const { return position_; }

private:
    s32    width_    = 0;                          //!< 配列幅
    s32    height_   = 0;                          //!< 配列高さ
    float3 position_ = float3(0.0f, 0.0f, 0.0f);   //!< 位置
    f32    scale_    = 1.0f;                       //!< 全体スケール
    f32    speed_    = 8.0f;                       //!< 速度スケール

    std::vector<float2> water_height_;    //!< 水面ハイトマップ配列
    std::vector<float2> water_height2_;   //!< 水面ハイトマップ配列
};

DEBUG_OPTIMIZE_OFF

//! 水面実体
std::unique_ptr<Water> water_;

//! 視錐台
Frustum frustum_;

}   // namespace

//---------------------------------------------------------------------------
//! 初期化
//---------------------------------------------------------------------------
bool SceneWater::Init()
{
    // グリッド表示をOFF
    ShowGrid(false);

    auto obj = Scene::CreateObject<Object>();

    //----------------------------------------------------------
    // モデルコンポーネント
    //----------------------------------------------------------
    model_ = std::make_shared<Model>("data/Sample/Pool/pool.mv1");

    //----------------------------------------------------------
    // カメラ
    //----------------------------------------------------------
    frustum_.setPosition(float3(-6.0f, 4.0f, -2.0f));   // 位置
    frustum_.setLookAt(float3(0.0f, 0.0f, -8.0f));      // 注視点
    frustum_.setFov(60.0f * DegToRad);                  // 画角
    frustum_.setNearZ(0.01f);                           // 近クリップ
    frustum_.setFarZ(100.0f);                           // 遠クリップ
    frustum_.update();

    SetCameraViewMatrix(cast(frustum_.matView()));            // ビュー行列
    SetupCamera_ProjectionMatrix(cast(frustum_.matProj()));   // 投影行列

    //==========================================================
    // 水面を初期化
    //==========================================================
    {
        s32    w        = 82;                                // 配列幅
        s32    h        = 146;                               // 配列高さ
        float3 position = float3(-4.625f, -0.5f, -21.25f);   // 位置
        f32    scale    = 1.0f / 8.0f;                       // 全体スケール

        water_ = std::make_unique<Water>(w, h, position, scale);
    }

    return true;
}

//---------------------------------------------------------------------------
//! 更新
//! @param  [in]    delta   経過時間
//---------------------------------------------------------------------------
void SceneWater::Update([[maybe_unused]] float delta)
{
    //----------------------------------------------------------
    // モデル更新
    //----------------------------------------------------------
    matrix mat_world = matrix::scale(0.1f);
    model_->setWorldMatrix(mat_world);

    model_->update(delta);

    //==========================================================
    // 水面更新
    //==========================================================

    //---- 水面をリセット
    if(IsKeyOn(KEY_INPUT_Z)) {
        water_->reset();
    }

    //---- ランダムに波紋を起こす
    if(IsKeyOn(KEY_INPUT_SPACE)) {
        auto w = water_->width();
        auto h = water_->height();

        // 外周部を書き換えないように1個内側の範囲で指定
        s32 x = rand() % (w - 2);
        s32 y = rand() % (h - 2);

        water_->water(x + 1, y + 1) += 1.0f;
    }

    //---- マウスクリックした場所に波紋を起こす
    if(IsMouseRepeat(MOUSE_INPUT_LEFT)) {   // 左クリックしている間
        // マウス座標を取得
        s32 x, y;
        GetMousePoint(&x, &y);

        // マウス座標から画面のスクリーン座標を計算
        float2 uv              = float2(static_cast<f32>(x) / WINDOW_W, static_cast<f32>(y) / WINDOW_H);
        float2 screen_position = (uv - 0.5f) * float2(2.0f, -2.0f);   // UV座標系 → スクリーン座標系への変換

        // クリック先へ向かうベクトルを計算
        float3 ray = frustum_.rayFromScreenPosition(screen_position);

        //------------------------------------------------------
        // 水面と直線の交点を求める
        //------------------------------------------------------
        // 直線の式 P' = (E - S) * t + S
        //    water.y = ray * t + カメラ位置

        // 水面の高さ y=-0.5f とtの関係式を作成
        f32 t = (water_->position().y - frustum_.position().y) / ray.y;

        // クリックされた水面の座標
        float3 water_position = ray * t + frustum_.position();

        //------------------------------------------------------
        // 水面に波を起こす
        // 表面の座標では高さ判定の誤差で反応しない場合があるため
        // 水面よりも少し下の位置を指定
        //------------------------------------------------------
        bool result = water_->addImpulse(water_position + float3(0.0f, water_->position().y - 0.5f, 0.0f), 0.125f);

        if(result) {
            // クリック先をデバッグ表示(赤色の球を表示)
            SetUseLighting(false);
            DrawSphere3D(cast(water_position), 0.25f, 8, GetColor(255, 0, 0), 0, false);
            SetUseLighting(true);
        }
    }

    // シミュレーション更新
    water_->update(delta);
}

//---------------------------------------------------------------------------
//! 描画
//---------------------------------------------------------------------------
void SceneWater::Draw()
{
    //----------------------------------------------------------
    // モデル描画
    //----------------------------------------------------------
    model_->render();

    //==========================================================
    // 水面描画
    //==========================================================
    water_->render();

    //----------------------------------------------------------
    // 操作説明を描画
    //----------------------------------------------------------
    {
        // 書式付き文字列の描画幅・高さ・行数を取得する
        const char* message =
            "[SPACE] ランダムで波紋を起こす   [L-click] クリックした場所に波紋を起こす [Z] 波をリセット";
        s32 width;        // 幅
        s32 height;       // 高さ
        s32 line_count;   // 行数
        GetDrawFormatStringSize(&width, &height, &line_count, message);

        // センタリング
        s32 x = (WINDOW_W - width) / 2;
        s32 y = WINDOW_H - height - 32;

        DrawFormatString(x, y, GetColor(255, 255, 255), message);
    }
}

//---------------------------------------------------------------------------
//! 終了
//---------------------------------------------------------------------------
void SceneWater::Exit()
{
    // グリッド表示をON
    ShowGrid(true);

    // 解放
    water_.reset();
}

//---------------------------------------------------------------------------
//! GUI表示
//---------------------------------------------------------------------------
void SceneWater::GUI()
{
}
