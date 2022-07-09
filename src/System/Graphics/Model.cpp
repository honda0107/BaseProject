//---------------------------------------------------------------------------
//! @file   Model.cpp
//! @brief  3Dモデル
//---------------------------------------------------------------------------
#include "Model.h"
#include "ResourceModel.h"
#include "ModelCache.h"
#include "Shader.h"
#include "Animation.h"

namespace
{

//! モデルリソースプール
std::unordered_map<std::string, std::shared_ptr<ResourceModel>> resource_model_pool;

}   // namespace

//---------------------------------------------------------------------------
//! 読み込み
//---------------------------------------------------------------------------
bool Model::load(std::string_view path)
{
    //----------------------------------------------------------
    // モデルリソース読み込み
    //----------------------------------------------------------
    {
        std::string resource_path = std::string(path);

        auto it = resource_model_pool.find(resource_path);
        if(it == resource_model_pool.end()) {   // 新規登録
            resource_model_pool[resource_path] = std::make_shared<ResourceModel>(path);
        }

        // 共有
        resource_model_ = resource_model_pool[resource_path];
    }

    // 初回のみ読み込み
    if(ref_counter_ == 0) {
        //----------------------------------------------------------
        // シェーダーを読み込み
        //----------------------------------------------------------
        // 頂点シェーダー
        // DX_MV1_VERTEX_TYPE_NUM 個のシェーダーバリエーションを生成
        shader_vs_ = std::make_shared<ShaderVs>("data/Shader/vs_model", VS_VARIANT_COUNT);

        // ピクセルシェーダー
        shader_ps_ = std::make_shared<ShaderPs>("data/Shader/ps_model", PS_VARIANT_COUNT);

        //----------------------------------------------------------
        // デフォルトテクスチャを読み込み
        //----------------------------------------------------------
        tex_null_white_  = std::make_shared<Texture>("data/System/null_white.dds");
        tex_null_black_  = std::make_shared<Texture>("data/System/null_black.dds");
        tex_null_normal_ = std::make_shared<Texture>("data/System/null_normal.dds");
    }
    ref_counter_++;

    return resource_model_->isValid();
}

//---------------------------------------------------------------------------
//! 更新
//---------------------------------------------------------------------------
void Model::update([[maybe_unused]] f32 dt)
{
}

//---------------------------------------------------------------------------
//! 描画
//---------------------------------------------------------------------------
void Model::render(ShaderVs* override_vs, ShaderPs* override_ps)
{
    if(!resource_model_->isActive()) {
        // ロードが終わっていない間は軽量モデルキャッシュ側を描画
        auto* model_cache = resource_model_->modelCache();
        model_cache->render(mat_world_);

        return;
    }

    // ロード終了していたらハンドルを複製
    on_initialize();

    for(s32 mesh = 0; mesh < MV1GetMeshNum(mv1_handle_); ++mesh) {   // モデルに含まれるメッシュの数
        renderByMesh(mesh, override_vs, override_ps);
    }
}

//---------------------------------------------------------------------------
//! メッシュ番号指定で描画
//---------------------------------------------------------------------------
void Model::renderByMesh(s32 mesh, ShaderVs* override_vs, ShaderPs* override_ps)
{
    if(!resource_model_->isActive())
        return;

    // ロード終了していたらハンドルを複製
    on_initialize();

    // メッシュ番号が有効範囲外
    if(MV1GetMeshNum(mv1_handle_) <= mesh) {
        return;
    }

    // ワールド行列を設定
    MV1SetMatrix(mv1_handle_, mat_world_);

    // シェーダーを使わない場合はDxLib関数を直接実行
    if(!use_shader_) {
        MV1DrawMesh(mv1_handle_, mesh);
        return;
    }

    //--------------------------------------------------
    // テクスチャ設定の上書き
    //--------------------------------------------------
    bool override_normalmap = false;

    if(overridedTextures_[static_cast<s32>(Model::TextureType::Diffuse)]) {
        SetUseTextureToShader(0, *overridedTextures_[static_cast<s32>(Model::TextureType::Diffuse)]);
    }
    if(overridedTextures_[static_cast<s32>(Model::TextureType::Normal)]) {
        SetUseTextureToShader(1, *overridedTextures_[static_cast<s32>(Model::TextureType::Normal)]);
        override_normalmap = true;   // 法線マップを使用
    }
    if(overridedTextures_[static_cast<s32>(Model::TextureType::Specular)]) {
        SetUseTextureToShader(2, *overridedTextures_[static_cast<s32>(Model::TextureType::Specular)]);
    }

    //--------------------------------------------------
    // シェーダーで描画
    //--------------------------------------------------

    ShaderVs* vs = override_vs ? override_vs : shader_vs_.get();
    ShaderPs* ps = override_ps ? override_ps : shader_ps_.get();

    // オリジナルシェーダーを使用をONにする
    MV1SetUseOrigShader(true);

    for(s32 t = 0; t < MV1GetMeshTListNum(mv1_handle_, mesh); ++t) {   // メッシュに含まれるトライアングルリストの数

        // トライアングルリスト番号
        auto tlist = MV1GetMeshTList(mv1_handle_, mesh, t);

        // トライアングルリストが使用しているマテリアルのインデックスを取得する
        auto material_index = MV1GetTriangleListUseMaterial(mv1_handle_, tlist);

        // 法線マップを使用しているかどうか
        bool use_normalmap = MV1GetMaterialNormalMapTexture(mv1_handle_, material_index) != -1;

        // 法線マップを使用しない場合はNull法線を登録しておく
        if(!use_normalmap && !override_normalmap) {
            SetUseTextureToShader(1, *tex_null_normal_);
        }

        //--------------------------------------------------
        // シェーダーバリエーションを選択
        //--------------------------------------------------
        // 頂点データタイプ(DX_MV1_VERTEX_TYPE_1FRAME 等)
        auto vertex_type = MV1GetTriangleListVertexType(mv1_handle_, tlist);

        u32 variant_vs = vertex_type;   // DXライブラリの頂点タイプをそのままバリエーション番号に

        //--------------------------------------------------
        // トライアングルリストを描画
        //--------------------------------------------------
        int handle_vs = vs->variant(variant_vs);
        int handle_ps = *ps;

        // シェーダーがない場合はオリジナルシェーダー利用を無効化
        bool shaderEnable = (handle_vs != -1) && (handle_ps != -1);
        MV1SetUseOrigShader(shaderEnable);

        // 頂点シェーダー
        SetUseVertexShader(handle_vs);

        // ピクセルシェーダー
        SetUsePixelShader(handle_ps);

        // 描画
        MV1DrawTriangleList(mv1_handle_, tlist);
    }

    // オリジナルシェーダー使用をOFFにする
    MV1SetUseOrigShader(false);

    // 使い終わったらテクスチャ設定を解除
    // 解除しておかないと他のモデル描画に影響あり。
    SetUseTextureToShader(0, -1);
    SetUseTextureToShader(1, -1);
    SetUseTextureToShader(2, -1);
}

//---------------------------------------------------------------------------
//  コンストラクタ
//---------------------------------------------------------------------------
Model::Model(std::string_view path)
{
    load(path);
}

//---------------------------------------------------------------------------
//  デストラクタ
//---------------------------------------------------------------------------
Model::~Model()
{
    // アニメーションが関連付けられている場合は相手の設定を解除
    if(animation_) {
        animation_->bindModel(nullptr);
    }

    // [DxLib] MV1モデルを解放 (複製されたハンドル)
    if(mv1_handle_ != -1) {
        MV1DeleteModel(mv1_handle_);
    }

    // 一番最後のオブジェクトが解放を担当
    ref_counter_--;
    if(ref_counter_ == 0) {
        // シェーダー
        shader_vs_.reset();
        shader_ps_.reset();

        // デフォルトテクスチャ
        tex_null_white_.reset();
        tex_null_black_.reset();
        tex_null_normal_.reset();
    }
}

//---------------------------------------------------------------------------
//! アニメーションを設定
//---------------------------------------------------------------------------
void Model::bindAnimation(Animation* animation)
{
    if(animation == animation_)
        return;

    // 旧モデルを解除
    auto* last = animation_;
    if(last) {
        animation_ = nullptr;
        last->bindModel(nullptr);
    }

    // 新モデルに登録
    animation_ = animation;
    if(animation_) {
        animation_->bindModel(this);
    }
}

//---------------------------------------------------------------------------
//!  既存テクスチャをオーバーライドします
//---------------------------------------------------------------------------
void Model::overrideTexture(Model::TextureType type, std::shared_ptr<Texture>& texture)
{
    overridedTextures_[static_cast<s32>(type)] = texture;
}

//---------------------------------------------------------------------------
//! ワールド行列を設定
//---------------------------------------------------------------------------
matrix Model::worldMatrix() const
{
    return mat_world_;
};

//---------------------------------------------------------------------------
//! [DxLib] MV1ハンドルを取得
//---------------------------------------------------------------------------
Model::operator int()
{
    // 強制的にハンドルを複製 (ブロッキングロードに切り替わる)
    on_initialize();

    return mv1_handle_;
}

//---------------------------------------------------------------------------
//! ファイルパスを取得
//---------------------------------------------------------------------------
const std::wstring& Model::path() const
{
    return path_;
}

//---------------------------------------------------------------------------
//! 初期化が正しく成功しているかどうかを取得
//---------------------------------------------------------------------------
bool Model::isValid() const
{
    return resource_model_->isValid();
}

//---------------------------------------------------------------------------
//! 利用可能な状態かどうか取得
//---------------------------------------------------------------------------
bool Model::isActive() const
{
    return resource_model_->isActive();
}

//---------------------------------------------------------------------------
//! モデルリソースを取得
//---------------------------------------------------------------------------
ResourceModel* Model::resource() const
{
    return resource_model_.get();
}

//---------------------------------------------------------------------------
//! 遅延初期化
//---------------------------------------------------------------------------
void Model::on_initialize()
{
    if(need_initialize_ == false)
        return;

    if(mv1_handle_ == -1) {
        mv1_handle_ = MV1DuplicateModel(*resource_model_);   // ハンドルを複製
    }

    need_initialize_ = false;
}
