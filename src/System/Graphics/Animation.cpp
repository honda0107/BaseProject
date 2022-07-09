//---------------------------------------------------------------------------
//! @file   Animation.cpp
//! @brief  アニメーション
//---------------------------------------------------------------------------
#include "Animation.h"
#include "System/EaseCurve.h"

namespace
{

//! アニメーションリソースプール
std::unordered_map<std::string, std::shared_ptr<ResourceAnimation>> resource_pool;

}   // namespace

//---------------------------------------------------------------------------
//! コントラクタ
//---------------------------------------------------------------------------
Animation::Animation(const Animation::Desc* desc, size_t desc_count)
{
    load(desc, desc_count);
}

//---------------------------------------------------------------------------
//! デストラクタ
//---------------------------------------------------------------------------
Animation::~Animation()
{
    // モデルが関連付けられている場合は相手の設定を解除
    if(model_) {
        model_->bindAnimation(nullptr);
    }

    // アニメーションMV1を解放
    for(auto& x : mv1_handles_) {
        if(x == -1)
            continue;

        MV1DeleteModel(x);
        x = -1;
    }
}

//---------------------------------------------------------------------------
//! 作成
//---------------------------------------------------------------------------
bool Animation::load(const Animation::Desc* desc, size_t desc_count)
{
    is_valid_ = true;   // 先に一旦trueにする

    // パラメーター初期化
    for(u32 i = 0; i < desc_count; ++i) {
        const auto& x = desc[i];

        //------------------------------------------------------
        // アニメーションリソース作成
        // 既に同名ファイルがある場合は共有
        //------------------------------------------------------
        const std::string& resource_path = x.file_path_;

        auto it = resource_pool.find(resource_path);
        if(it == resource_pool.end()) {   // 新規登録
            resource_pool[resource_path] = std::make_shared<ResourceAnimation>(resource_path);
        }

        // 共有
        std::shared_ptr<ResourceAnimation>& resource = resource_pool[resource_path];

        descs_.push_back(x);
        mv1_handles_.push_back(MV1DuplicateModel(*resource));

        if(resource->isValid() == false) {
            is_valid_ = false;   // 一度でもエラーの場合はfalse
        }

        // 名前逆引きテーブルに登録
        name_table_[desc[i].name_] = i;
    }

    return is_valid_;
}

//---------------------------------------------------------------------------
//! 更新
//---------------------------------------------------------------------------
void Animation::update(f32 dt)
{
    assert(model_ && "アニメーションにモデルが設定されていません");

    // 再生していない場合
    if(isPlaying() == false)
        return;

    // ポーズ中の場合
    if(is_paused_)
        return;

    //----------------------------------------------------------
    // アニメーションを進める
    //----------------------------------------------------------
    for(u32 i = 0; i < 2; ++i) {
        auto& c = contexts_[i];

        if(c.animation_index_ == -1)
            continue;

        if(c.is_playing_ == false)
            continue;

        auto& desc = descs_[c.animation_index_];

        // アニメーション再生時間を進める
        c.play_time_ += dt * 30.0f * desc.animation_speed_;

        if(c.is_loop_) {
            // アニメーション再生時間がアニメーションの総時間を越えていたらループさせる
            if(c.animation_total_time_ <= c.play_time_) {
                c.play_time_ -= c.animation_total_time_;
            }
        }
        else {
            // アニメーション再生時間がアニメーションの総時間を越えていたら停止
            if(c.animation_total_time_ <= c.play_time_) {
                c.play_time_  = c.animation_total_time_;
                c.is_playing_ = false;
            }
        }

        // 新しいアニメーション再生時間をセット
        MV1SetAttachAnimTime(model_handle_, c.animation_attach_index_, c.play_time_);
    }

    //----------------------------------------------------------
    // アニメーションブレンド
    //----------------------------------------------------------

    // ブレンドを進める
    blend_ratio_ = std::max(0.0f, blend_ratio_ - 1.0f / blend_time_ * dt);

    // 線形で等速補間すると硬い動きになるためEaseカーブ補間
    auto ease  = GetEaseFunction(EaseType::InOutCubic);   // 加減速
    f32  ratio = ease(blend_ratio_);

    // ブレンド比率を設定
    MV1SetAttachAnimBlendRate(model_handle_, contexts_[0].animation_attach_index_, 1.0f - ratio);
    MV1SetAttachAnimBlendRate(model_handle_, contexts_[1].animation_attach_index_, ratio);

    // 補間完了後は補間元のアニメーションを停止
    if(blend_ratio_ == 0.0f) {   // float値の==判定は0.0fのみ正確に判定できる
        detachAnimation(1);
        contexts_[1].is_playing_ = false;
    }
}

//---------------------------------------------------------------------------
//! アニメーション再生するモデルを設定する
//---------------------------------------------------------------------------
void Animation::bindModel(Model* model)
{
    if(model == model_)
        return;

    // 旧モデルを解除
    auto* last = model_;
    if(last) {
        // [DxLib]モデルとアニメーションの関連付けを解除
        detachAnimation(0);
        detachAnimation(1);

        model_        = nullptr;
        model_handle_ = -1;

        last->bindAnimation(nullptr);
    }

    // 新モデルに登録
    model_ = model;
    if(model_) {
        model_handle_ = *model_;
        model_->bindAnimation(this);
    }
}

//---------------------------------------------------------------------------
//! アニメーションを再生する
//---------------------------------------------------------------------------
bool Animation::play(std::string_view name, bool is_loop, f32 blend_time, f32 start_time)
{
    // ブレンドの速度
    blend_time_ = std::max(FLT_EPSILON, blend_time);   // 0.0fにならないように

    //----------------------------------------------------------
    // [DxLib] 設定されているアニメーションを一旦解除
    //----------------------------------------------------------
    detachAnimation(0);
    detachAnimation(1);

    // 補間のために現在のアニメーションを補間前として設定
    // [0] = 現在のアニメーション
    // [1] = 以前のアニメーション
    contexts_[1] = contexts_[0];

    //----------------------------------------------------------
    // 新規再生
    //----------------------------------------------------------
    {
        // 名前からアニメーションを検索
        std::string name_string(name);
        auto        it = name_table_.find(name_string);

        if(it == name_table_.end()) {
            return false;   // 見つからなかった場合
        }

        auto& c = contexts_[0];

        // アニメーション番号
        c.animation_index_ = it->second;
        // 現在の時間
        c.play_time_ = start_time;   // 開始位置は start_time から

        c.is_playing_ = true;
        c.is_loop_    = is_loop;
    }

    //----------------------------------------------------------
    // アニメーション再生を設定
    //----------------------------------------------------------
    for(u32 i = 0; i < 2; ++i) {
        attachAnimation(i);
    }

    // 補間対象のアニメーションがある場合は補間開始
    if(contexts_[1].animation_index_ != -1) {
        blend_ratio_ = 1.0f;
    }

    // ポーズ解除
    is_paused_ = false;

    return true;
}

//---------------------------------------------------------------------------
//! アニメーションを停止する
//---------------------------------------------------------------------------
void Animation::pause(bool active)
{
    is_paused_ = active;
}

//---------------------------------------------------------------------------
//!  再生中かどうかを取得
//---------------------------------------------------------------------------
bool Animation::isPlaying() const
{
    return contexts_[0].is_playing_;
}

//---------------------------------------------------------------------------
//!  一時停止中かどうかを取得
//---------------------------------------------------------------------------
bool Animation::isPaused() const
{
    return is_paused_;
}

//---------------------------------------------------------------------------
//! 初期化が正しく成功しているかどうかを取得
//---------------------------------------------------------------------------
bool Animation::isValid() const
{
    return is_valid_;
}

//---------------------------------------------------------------------------
//! 利用可能な状態かどうか取得
//---------------------------------------------------------------------------
bool Animation::isActive() const
{
    return isValid();
}

//---------------------------------------------------------------------------
//! アニメーションを割り当て
//---------------------------------------------------------------------------
bool Animation::attachAnimation(s32 context_index)
{
    auto& c = contexts_[context_index];

    if(c.animation_index_ == -1)
        return false;

    auto& desc = descs_[c.animation_index_];

    // モデルにアニメーションを設定
    auto animation_index = desc.animation_index_;              // アニメーション番号
    auto mv1_handle      = mv1_handles_[c.animation_index_];   // [DxLib] アニメーションのMV1

    c.animation_attach_index_ = MV1AttachAnim(model_handle_, animation_index, mv1_handle, true);

    // アニメーション総再生時間を取得
    c.animation_total_time_ = MV1GetAttachAnimTotalTime(model_handle_, c.animation_attach_index_);

    // アニメーション再生時間を初期化
    MV1SetAttachAnimTime(model_handle_, c.animation_attach_index_, c.play_time_);

    return true;
}

//---------------------------------------------------------------------------
//! アニメーション割り当てを解除
//---------------------------------------------------------------------------
void Animation::detachAnimation(s32 context_index)
{
    MV1DetachAnim(model_handle_, contexts_[context_index].animation_attach_index_);
    contexts_[context_index].animation_attach_index_ = -1;
}

//===========================================================================
//  ResourceAnimation
//===========================================================================

//---------------------------------------------------------------------------
//! コンストラクタ
//---------------------------------------------------------------------------
ResourceAnimation::ResourceAnimation(std::string_view path)
{
    //-----------------------------------------------------------------------
    // MV1モデルアニメーションの読み込み
    //-----------------------------------------------------------------------
    std::string resource_path(path);

    // パスの保存
    path_ = convertTo(resource_path);

    // アニメーションが含まれるモデルデータを読み込み
    mv1_handle_ = MV1LoadModel(resource_path.c_str());

    // ハンドルの非同期読み込み処理が完了したら呼ばれる関数
    auto finish_callback = []([[maybe_unused]] int mv1_handle, void* data) {
        auto* resource = reinterpret_cast<ResourceAnimation*>(data);

        // アクティブフラグを設定
        resource->active_ = true;
    };

    //----------------------------------------------------------
    // 非同期読み込み
    //----------------------------------------------------------
    SetUseASyncLoadFlag(true);
    {
        // モデルの読み込み
        mv1_handle_ = MV1LoadModel(resource_path.c_str());
        SetASyncLoadFinishCallback(mv1_handle_, (void (*)(int, void*))finish_callback, this);
    }
    SetUseASyncLoadFlag(false);
}

//---------------------------------------------------------------------------
//! デストラクタ
//---------------------------------------------------------------------------
ResourceAnimation::~ResourceAnimation()
{
    // アニメーションを解放
    if(mv1_handle_ != -1) {
        MV1DeleteModel(mv1_handle_);
    }
}

//---------------------------------------------------------------------------
//! 初期化が正しく成功しているかどうかを取得
//---------------------------------------------------------------------------
bool ResourceAnimation::isValid() const
{
    return mv1_handle_ != -1;
}

//---------------------------------------------------------------------------
//! 利用可能な状態かどうか取得
//---------------------------------------------------------------------------
bool ResourceAnimation::isActive() const
{
    return active_;
}

//---------------------------------------------------------------------------
//! [DxLib] MV1ハンドルを取得
//---------------------------------------------------------------------------
ResourceAnimation::operator int() const
{
    return mv1_handle_;
}
