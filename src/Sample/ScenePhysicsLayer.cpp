//---------------------------------------------------------------------------
//! @file   ScenePhysicsLayer.cpp
//! @brief  物理衝突レイヤー分けサンプル
//---------------------------------------------------------------------------
#include "ScenePhysicsLayer.h"
#include <System/Component/ComponentCamera.h>

#include <System/Physics/PhysicsEngine.h>
#include <System/Physics/PhysicsLayer.h>
#include <System/Physics/RigidBody.h>
#include <System/Physics/Shape.h>
#include <System/Physics/PhysicsCharacter.h>   // キャラクターコントローラー

BP_CLASS_IMPL(ScenePhysicsLayer, u8"[Physics] 衝突レイヤー分けサンプル");

namespace
{

//==============================================================
// カテゴリーレイヤーを定義
// 種類や番号は任意に作成可能です
//==============================================================
enum MyLayer : u16
{
    Player,   // プレイヤー
    Prop0,    // 設置物0
    Prop1,    // 設置物1
    Prop2,    // 設置物2
    Prop3,    // 設置物3
    Static,   // 動かないオブジェクト
};

//==============================================================
// カテゴリーレイヤーの属性を適用する定義
// physics::ObjectLayersとカテゴリーレイヤーを関連付けます
//==============================================================
std::pair<physics::ObjectLayers, u16> myLayerTable_[] = {
    {    physics::ObjectLayers::MOVING, MyLayer::Player}, // プレイヤー
    {    physics::ObjectLayers::MOVING,  MyLayer::Prop0}, // 設置物1
    {    physics::ObjectLayers::MOVING,  MyLayer::Prop1}, // 設置物1
    {    physics::ObjectLayers::MOVING,  MyLayer::Prop2}, // 設置物2
    {    physics::ObjectLayers::MOVING,  MyLayer::Prop3}, // 設置物3
    {physics::ObjectLayers::NON_MOVING, MyLayer::Static}, // 動かないオブジェクト
};

//==============================================================
// 2つのカテゴリーレイヤーが衝突可能かどうかを判断するカスタム関数
//==============================================================
bool MyObjectCanCollide(u16 object1, u16 object2)
{
    switch(object1) {
    case MyLayer::Player:   // プレイヤーはProp0と衝突しない
        return object2 != MyLayer::Prop0;
    case MyLayer::Prop0:   // 設置物0は床のみ衝突
        return object2 == MyLayer::Static;
    case MyLayer::Prop1:   // 設置物1
    case MyLayer::Prop2:   // 設置物2
    case MyLayer::Prop3:   // 設置物3
        // 他の全てと衝突
        return true;
    case MyLayer::Static:   // 動かないオブジェクト
        // 他の全てと衝突
        return true;
    default:
        assert(false && "定義されていないレイヤーです");
        break;
    }
    return true;
};

//==============================================================
//! キャラクターコンタクトリスナーサンプル
//==============================================================

//！ 設置物のボディIDリスト
std::vector<u64> prop_body_id_list_;

class CharacterContactListener : public physics::Character::ContactListener
{
    //  キャラクターが指定されたボディと衝突可能かどうかをチェックします。
    //! @param  [in]    character       対象のキャラクター
    //! @param  [in]    other_body_id   相手のボディID
    //! @retval true    衝突可能
    //! @retval false   衝突しない
    virtual bool onContactValidate([[maybe_unused]] const physics::Character* character,
                                   [[maybe_unused]] u64                       other_body_id)
    {
        // デフォルトは常に衝突許可
        return true;
    }

    //  キャラクターがボディと衝突するたびに呼び出されます
    //! @param  [in]    character           対象のキャラクター
    //! @param  [in]    other_body_id       相手のボディID
    //! @param  [in]    contact_position    衝突位置
    //! @param  [in]    contact_normal      衝突法線
    //! @param  [out]   result              衝突許可情報
    virtual void onContactAdded([[maybe_unused]] const physics::Character*            character,
                                [[maybe_unused]] u64                                  other_body_id,
                                [[maybe_unused]] const float3&                        contact_position,
                                [[maybe_unused]] const float3&                        contact_normal,
                                [[maybe_unused]] physics::Character::ContactSettings& result)
    {
        auto it = std::find(prop_body_id_list_.begin(), prop_body_id_list_.end(), other_body_id);
        if(it != prop_body_id_list_.end()) {
            size_t index = it - prop_body_id_list_.begin();   // 配列インデックス番号

            switch(index) {
            case 0:
                result.can_push_character_   = false;   // 押されない
                result.can_receive_impulses_ = false;   // オブジェクトを押せない
                break;
            case 1:
                result.can_push_character_   = false;   // 押されない
                result.can_receive_impulses_ = true;    // オブジェクトを押せる
                break;
            case 2:
                result.can_push_character_   = true;   // 押される
                result.can_receive_impulses_ = true;   // オブジェクトを押せる
                break;
            case 3:
                result.can_push_character_   = true;    // 押される
                result.can_receive_impulses_ = false;   // オブジェクトを押せない
                break;
            }
        }
    }
};

CharacterContactListener listener_;

}   // namespace

//---------------------------------------------------------------------------
//! 初期化
//---------------------------------------------------------------------------
bool ScenePhysicsLayer::Init()
{
    //==========================================================
    // オブジェクトレイヤーをカスタム設定
    //==========================================================
    physics::Engine::instance()->setLayerAlias(myLayerTable_, std::size(myLayerTable_));
    physics::Engine::instance()->overrideLayerCollide(MyObjectCanCollide);
    //----------------------------------------------------------
    //  物理シミュレーションをリセット
    //----------------------------------------------------------
    ResetPhysics();

    //----------------------------------------------------------
    // カメラコンポーネント
    //----------------------------------------------------------
    auto obj = Scene::CreateObject<Object>();

    auto camera = obj->AddComponent<ComponentCamera>();
    camera->SetPerspective(75.0f);   // 画角
    camera->SetPositionAndTarget(float3(0.0f, 2.0f, -10.0f), {0.0f, 1.0f, 0.0f});
    camera->SetCurrentCamera();

    return true;
}

//---------------------------------------------------------------------------
//! 更新
//! @param  [in]    delta   経過時間
//---------------------------------------------------------------------------
void ScenePhysicsLayer::Update([[maybe_unused]] float delta)
{
    auto* physicsEngine = physics::Engine::instance();

    // 更新前の座標を保存
    float3 old_position = character_->position();

    //----------------------------------------------------------
    // キャラクターを更新
    //----------------------------------------------------------
    {
        character_->update(delta);

        //----------------------------------------------------------
        // 操作
        //----------------------------------------------------------
        float3 move_direction(0.0f, 0.0f, 0.0f);   // 移動ベクトル
        f32    character_speed_ = 1.0f;            // キャラクターの移動速度スケール

        if(IsKeyRepeat(KEY_INPUT_A)) {
            move_direction.x -= 1.0f;
        }
        if(IsKeyRepeat(KEY_INPUT_D)) {
            move_direction.x += 1.0f;
        }

        if(IsKeyRepeat(KEY_INPUT_W)) {
            move_direction.z += 1.0f;
        }
        if(IsKeyRepeat(KEY_INPUT_S)) {
            move_direction.z -= 1.0f;
        }

        // 移動方向を正規化
        if(dot(move_direction, move_direction).x != 0.0f) {
            move_direction = normalize(move_direction) * 10.0f;
        }

        // 移動と背景衝突補正
        float3 jump_direction = float3(0.0f, 0.0f, 0.0f);
        float3 new_velocity   = character_->move(delta, move_direction, old_position, jump_direction);

        //------------------------------------------------------
        // 速度の反映
        //------------------------------------------------------

        // 重力
        new_velocity += physicsEngine->gravity() * delta;

        // プレイヤー操作
        new_velocity += move_direction * character_speed_;

        // 速度を更新
        character_->setLinearVelocity(new_velocity);
    }

    //----------------------------------------------------------
    //  スペースキーで物理シミュレーションをリセット
    //----------------------------------------------------------
    if(GetKeyState(VK_SPACE) & 0x8000) {
        ResetPhysics();
    }

    //----------------------------------------------------------
    // 剛体の姿勢をモデルに設定
    //----------------------------------------------------------
    {
        matrix mat_world = matrix::scale(0.5f);

        // 剛体からワールド行列を作成
        mat_world = mul(mat_world, rigid_body0_->worldMatrix());
        model_box1_->setWorldMatrix(mat_world);
    }
    {
        matrix mat_world = matrix::scale(1.0f);

        // 剛体からワールド行列を作成
        mat_world = mul(mat_world, rigid_body1_->worldMatrix());
        model_box2_->setWorldMatrix(mat_world);
    }
    {
        matrix mat_world = matrix::scale(1.0f);

        // 剛体からワールド行列を作成
        mat_world = mul(mat_world, rigid_body2_->worldMatrix());
        model_barrel_->setWorldMatrix(mat_world);
    }
    {
        matrix mat_world = matrix::scale(0.01f);

        // 剛体からワールド行列を作成
        mat_world = mul(mat_world, rigid_body3_->worldMatrix());
        model_cone_->setWorldMatrix(mat_world);
    }
}

//---------------------------------------------------------------------------
//! 描画
//---------------------------------------------------------------------------
void ScenePhysicsLayer::Draw()
{
    //----------------------------------------------------------
    // モデル
    //----------------------------------------------------------
    model_box1_->render();
    model_box2_->render();
    model_barrel_->render();
    model_cone_->render();

    DrawFormatString(100, 50, GetColor(255, 255, 255), "[Physic] 衝突レイヤー分けサンプル");

    //----------------------------------------------------------
    // キャラクターを描画
    //----------------------------------------------------------
    auto* shape = character_->shape();
    auto  s     = static_cast<shape::Capsule*>(shape);   // カプセル形状

    float3 position = character_->position();
    DrawCapsule3D(cast(position + float3(0.0f, 1.f, 0.0f) * s->radius_),
                  cast(position + float3(0.0f, 1.f, 0.0f) * (s->radius_ + s->half_height_ * 2.0f)),
                  s->radius_,
                  8,
                  GetColor(0, 255, 0),
                  GetColor(0, 0, 0),
                  true);

    //----------------------------------------------------------
    // 設置物の状態を表示
    //----------------------------------------------------------
    auto put_state = [](const float3& position, const char* message, u32 color) {
        // 書式付き文字列の描画幅・高さ・行数を取得する
        s32 width;        // 幅
        s32 height;       // 高さ
        s32 line_count;   // 行数
        GetDrawFormatStringSize(&width, &height, &line_count, message);

        // 2D表示位置を計算

        auto screen_position = ConvWorldPosToScreenPos(cast(position));

        // 位置の上側に表示

        s32 x = static_cast<s32>(screen_position.x) - width / 2;   // センタリング
        s32 y = static_cast<s32>(screen_position.y) - height;      // 上詰め

        DrawFormatString(x - 1, y - 1, GetColor(0, 0, 0), message);
        DrawFormatString(x + 1, y - 1, GetColor(0, 0, 0), message);
        DrawFormatString(x - 1, y + 1, GetColor(0, 0, 0), message);
        DrawFormatString(x + 1, y + 1, GetColor(0, 0, 0), message);
        DrawFormatString(x, y, color, message);
    };

    put_state(rigid_body0_->position() + float3(0.0f, 1.0f, 0.0f), "押されない\n押せない", GetColor(255, 255, 255));
    put_state(rigid_body1_->position() + float3(0.0f, 1.0f, 0.0f), "押されない\n押せる", GetColor(255, 255, 255));
    put_state(rigid_body2_->position() + float3(0.0f, 1.0f, 0.0f), "押される\n押せる", GetColor(255, 255, 255));
    put_state(rigid_body3_->position() + float3(0.0f, 1.0f, 0.0f), "押される\n押せない", GetColor(255, 255, 255));

    put_state(rigid_body0_->position() + float3(0.0f, 2.5f, 0.0f), "当たらない", GetColor(255, 64, 64));
    put_state(rigid_body1_->position() + float3(0.0f, 2.5f, 0.0f), "当たる", GetColor(64, 255, 64));
    put_state(rigid_body2_->position() + float3(0.0f, 2.5f, 0.0f), "当たる", GetColor(64, 255, 64));
    put_state(rigid_body3_->position() + float3(0.0f, 2.5f, 0.0f), "当たる", GetColor(64, 255, 64));

    //----------------------------------------------------------
    // 操作説明をデバッグ表示
    //----------------------------------------------------------
    {
        // 書式付き文字列の描画幅・高さ・行数を取得する
        const char* message = "[W/A/S/D] 移動 [SPACE] 物理シミュレーションをリセット";
        s32         width;        // 幅
        s32         height;       // 高さ
        s32         line_count;   // 行数
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
void ScenePhysicsLayer::Exit()
{
    //==========================================================
    // オブジェクトレイヤーカスタム設定を解除
    //==========================================================
    physics::Engine::instance()->setLayerAlias(nullptr, 0);
    physics::Engine::instance()->overrideLayerCollide(nullptr);
}

//---------------------------------------------------------------------------
//! GUI表示
//---------------------------------------------------------------------------
void ScenePhysicsLayer::GUI()
{
}

//---------------------------------------------------------------------------
//! 物理シミュレーションをリセット
//---------------------------------------------------------------------------
void ScenePhysicsLayer::ResetPhysics()
{
    //----------------------------------------------------------
    // オブジェクトを解放
    //----------------------------------------------------------
    body_floor_.reset();    // 床
    rigid_body1_.reset();   // 剛体
    rigid_body2_.reset();   // 剛体
    rigid_body3_.reset();   // 剛体

    //----------------------------------------------------------
    // モデル
    //----------------------------------------------------------

    // ボックス
    {
        std::string path = "data/Sample/Sci-fi_Box/Textures/";

        // PBRテクスチャで上書き
        auto tex_albedo_    = std::make_shared<Texture>(path + "DefaultMaterial_Base_color.png");
        auto tex_normal_    = std::make_shared<Texture>(path + "DefaultMaterial_Normal_DirectX.png");
        auto tex_roughness_ = std::make_shared<Texture>(path + "DefaultMaterial_Roughness.png");
        auto tex_metalness_ = std::make_shared<Texture>(path + "DefaultMaterial_Metallic.png");
        auto tex_ao_        = std::make_shared<Texture>(path + "DefaultMaterial_Mixed_AO.png");

        auto model = std::make_shared<Model>("data/Sample/Sci-fi_Box/Sci-fi Box.mv1");

        // モデルに設定されているテクスチャを上書き
        model->overrideTexture(Model::TextureType::Diffuse, tex_albedo_);
        model->overrideTexture(Model::TextureType::Normal, tex_normal_);
        model->overrideTexture(Model::TextureType::Roughness, tex_roughness_);
        model->overrideTexture(Model::TextureType::Metalness, tex_metalness_);
        model->overrideTexture(Model::TextureType::AO, tex_ao_);

        model_box1_ = std::move(model);
    }

    // コンテナボックス
    {
        std::string path = "data/Sample/Sci-fi_Container/Textures/";

        auto tex_albedo_    = std::make_shared<Texture>(path + "Sci-fi_Box_AlbedoTransparency.png");
        auto tex_normal_    = std::make_shared<Texture>(path + "Sci-fi_Box_Normal_DirectX.png");
        auto tex_roughness_ = std::make_shared<Texture>(path + "Sci-fi_Box_MetallicSmoothness.png");
        auto tex_metalness_ = std::make_shared<Texture>(path + "Sci-fi_Box_MetallicSmoothness.png");
        auto tex_ao_        = std::make_shared<Texture>(path + "Sci-fi_Box_AO.png");

        auto model = std::make_shared<Model>("data/Sample/Sci-fi_Container/Sci-fi Container.mv1");

        // モデルに設定されているテクスチャを上書き
        model->overrideTexture(Model::TextureType::Diffuse, tex_albedo_);
        model->overrideTexture(Model::TextureType::Normal, tex_normal_);
        model->overrideTexture(Model::TextureType::Roughness, tex_roughness_);
        model->overrideTexture(Model::TextureType::Metalness, tex_metalness_);
        model->overrideTexture(Model::TextureType::AO, tex_ao_);

        model_box2_ = std::move(model);
    }

    // ドラム缶
    {
        std::string path = "data/Sample/oil_barrels_pbr/textures/";

        auto tex_albedo_    = std::make_shared<Texture>(path + "drum2_base_color.png");
        auto tex_normal_    = std::make_shared<Texture>(path + "drum2_normal.png");
        auto tex_roughness_ = std::make_shared<Texture>(path + "drum2_roughness.png");
        auto tex_metalness_ = std::make_shared<Texture>(path + "drum2_metallic.png");
        auto tex_ao_        = std::make_shared<Texture>(path + "drum2_ambient.png");

        auto model = std::make_shared<Model>("data/Sample/oil_barrels_pbr/barrel.mv1");

        // モデルに設定されているテクスチャを上書き
        model->overrideTexture(Model::TextureType::Diffuse, tex_albedo_);
        model->overrideTexture(Model::TextureType::Normal, tex_normal_);
        model->overrideTexture(Model::TextureType::Roughness, tex_roughness_);
        model->overrideTexture(Model::TextureType::Metalness, tex_metalness_);
        model->overrideTexture(Model::TextureType::AO, tex_ao_);

        model_barrel_ = std::move(model);
    }

    // 三角コーン
    {
        std::string path = "data/Sample/Traffic_Cone/";

        auto tex_albedo_    = std::make_shared<Texture>(path + "Red_Dirty_Traffic Cone_Base_Color.png");
        auto tex_normal_    = std::make_shared<Texture>(path + "Dirty_Traffic Cone_Normal_DirectX.png");
        auto tex_roughness_ = std::make_shared<Texture>(path + "Dirty_Traffic Cone_Roughness.png");
        auto tex_metalness_ = std::make_shared<Texture>(path + "Dirty_Traffic Cone_Metallic.png");
        auto tex_ao_        = std::make_shared<Texture>(path + "Dirty_Traffic Cone_AO.png");

        auto model = std::make_shared<Model>("data/Sample/Traffic_Cone/Traffic Cone.mv1");

        // モデルに設定されているテクスチャを上書き
        model->overrideTexture(Model::TextureType::Diffuse, tex_albedo_);
        model->overrideTexture(Model::TextureType::Normal, tex_normal_);
        model->overrideTexture(Model::TextureType::Roughness, tex_roughness_);
        model->overrideTexture(Model::TextureType::Metalness, tex_metalness_);
        model->overrideTexture(Model::TextureType::AO, tex_ao_);

        model_cone_ = std::move(model);
    }

    //==========================================================
    // 剛体を作成
    //==========================================================
    rigid_body0_ = physics::createRigidBody(shape::Box{float3(0.5f, 0.5f, 0.5f)}, MyLayer::Prop0);
    rigid_body1_ = physics::createRigidBody(shape::Box{float3(0.5f, 0.5f, 0.5f)}, MyLayer::Prop1);
    rigid_body2_ = physics::createRigidBody(shape::Cylinder{0.5f, 0.375f}, MyLayer::Prop2);
    rigid_body3_ = physics::createRigidBody(shape::ConvexHull(model_cone_.get()), MyLayer::Prop3);

    // 座標設定
    rigid_body0_->setPosition(float3(-2.0f, 5.0f, 0.0f));
    rigid_body1_->setPosition(float3(2.0f, 5.0f, 0.0f));
    rigid_body2_->setPosition(float3(4.0f, 5.0f, 0.0f));
    rigid_body3_->setPosition(float3(6.0f, 5.0f, 0.0f));

    // 跳ね返り係数
    rigid_body0_->setRestitution(0.3f);
    rigid_body1_->setRestitution(0.3f);
    rigid_body2_->setRestitution(0.3f);
    rigid_body3_->setRestitution(0.3f);

    // ボディIDリストに登録
    prop_body_id_list_.clear();
    prop_body_id_list_.push_back(rigid_body0_->bodyID());
    prop_body_id_list_.push_back(rigid_body1_->bodyID());
    prop_body_id_list_.push_back(rigid_body2_->bodyID());
    prop_body_id_list_.push_back(rigid_body3_->bodyID());

    //==========================================================
    // 床を作成
    //==========================================================
    body_floor_ = physics::createRigidBody(shape::Box{float3(63.0f, 1.0f, 63.0f)},
                                           MyLayer::Static,                // [Layer] 動かないオブジェクト
                                           physics::MotionType::Static);   // 静的
    body_floor_->setPosition(float3(0.0f, -1.0f, 0.0f));

    //==========================================================
    // キャラクター
    //==========================================================
    character_ = physics::createCharacter(MyLayer::Player);   // [Layer] プレイヤー

    // リスナーを設定
    // キャラクターと他のオブジェクトとの干渉をカスタマイズできます
    character_->setListener(&listener_);

    // 形状を設定(必須)
    shape::Capsule shape_standing_ = shape::Capsule(0.8f, 0.3f);
    character_->setShape(shape_standing_, float3(0.0f, shape_standing_.half_height_ + shape_standing_.radius_, 0.0f));

    character_->setPosition(float3(0.0f, 0.0f, 0.0f));

    // シミュレーションを最適化
    // (この処理は必ず呼ばなければならないわけではありません)
    physics::Engine::instance()->optimize();
}
