//---------------------------------------------------------------------------
//! @file   ScenePhysics.cpp
//! @brief  物理シミュレーションサンプルシーン
//---------------------------------------------------------------------------
#include "ScenePhysics.h"
#include <System/Component/ComponentCamera.h>

#include <System/Physics/PhysicsEngine.h>
#include <System/Physics/PhysicsLayer.h>
#include <System/Physics/RigidBody.h>
#include <System/Physics/Shape.h>

BP_CLASS_IMPL(ScenePhysics, u8"[Physics] 物理シミュレーションサンプル");

//---------------------------------------------------------------------------
//! 初期化
//---------------------------------------------------------------------------
bool ScenePhysics::Init()
{
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
    camera->SetPositionAndTarget(float3(0.0f, 6.0f, -15.0f), {0.0f, 5.0f, 0.0f});
    camera->SetCurrentCamera();

    return true;
}

//---------------------------------------------------------------------------
//! 更新
//! @param  [in]    delta   経過時間
//---------------------------------------------------------------------------
void ScenePhysics::Update([[maybe_unused]] float delta)
{
    //----------------------------------------------------------
    //  スペースキーで物理シミュレーションをリセット
    //----------------------------------------------------------
    if(GetKeyState(VK_SPACE) & 0x8000) {
        ResetPhysics();
    }

    //----------------------------------------------------------
    // 剛体の姿勢をモデルに設定
    //----------------------------------------------------------
    for(u32 i = 0; i < rigid_bodies_.size(); ++i) {
        auto* rigid_body = rigid_bodies_[i].get();

        // モデルにワールド行列を設定
        switch(rigid_body->data()) {
        case 0:
            {
                matrix mat_world = matrix::scale(0.5f);

                // 剛体からワールド行列を作成
                mat_world = mul(mat_world, rigid_body->worldMatrix());
                model_boxes1_[i]->setWorldMatrix(mat_world);
            }
            break;
        case 1:
            {
                matrix mat_world = matrix::scale(1.0f);

                // 剛体からワールド行列を作成
                mat_world = mul(mat_world, rigid_body->worldMatrix());
                model_boxes2_[i]->setWorldMatrix(mat_world);
            }
            break;
        case 2:
            {
                matrix mat_world = matrix::scale(1.0f);

                // 剛体からワールド行列を作成
                mat_world = mul(mat_world, rigid_body->worldMatrix());
                model_barrel_[i]->setWorldMatrix(mat_world);
            }
            break;
        case 3:
            {
                matrix mat_world = matrix::scale(0.01f);

                // 剛体からワールド行列を作成
                mat_world = mul(mat_world, rigid_body->worldMatrix());
                model_cone_[i]->setWorldMatrix(mat_world);
            }
            break;
        default:
            break;
        }
    }
}

//---------------------------------------------------------------------------
//! 描画
//---------------------------------------------------------------------------
void ScenePhysics::Draw()
{
    // モデル
    for(u32 i = 0; i < rigid_bodies_.size(); ++i) {
        auto* rigid_body = rigid_bodies_[i].get();

        switch(rigid_body->data()) {
        case 0:
            model_boxes1_[i]->render();
            break;
        case 1:
            model_boxes2_[i]->render();
            break;
        case 2:
            model_barrel_[i]->render();
            break;
        case 3:
            model_cone_[i]->render();
            break;
        default:
            break;
        }
    }

    DrawFormatString(100, 50, GetColor(255, 255, 255), "Physics Demo");

    {
        // 書式付き文字列の描画幅・高さ・行数を取得する
        const char* message = "[SPACE] 物理シミュレーションをリセット";
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
void ScenePhysics::Exit()
{
}

//---------------------------------------------------------------------------
//! GUI表示
//---------------------------------------------------------------------------
void ScenePhysics::GUI()
{
}

//---------------------------------------------------------------------------
//! 物理シミュレーションをリセット
//---------------------------------------------------------------------------
void ScenePhysics::ResetPhysics()
{
    //----------------------------------------------------------
    // オブジェクトを解放
    //----------------------------------------------------------
    model_boxes1_.clear();   // ボックス
    model_boxes2_.clear();   // コンテナボックス
    model_barrel_.clear();   // ドラム缶
    model_cone_.clear();     // 三角コーン

    body_floor_.reset();     // 床
    rigid_bodies_.clear();   // 剛体

    //----------------------------------------------------------
    // モデル
    //----------------------------------------------------------
    constexpr s32 HEIGHT = 16;
    constexpr s32 WIDTH  = 15;

    // ボックス
    {
        std::string path = "data/Sample/Sci-fi_Box/Textures/";

        // PBRテクスチャで上書き
        auto tex_albedo_    = std::make_shared<Texture>(path + "DefaultMaterial_Base_color.png");
        auto tex_normal_    = std::make_shared<Texture>(path + "DefaultMaterial_Normal_DirectX.png");
        auto tex_roughness_ = std::make_shared<Texture>(path + "DefaultMaterial_Roughness.png");
        auto tex_metalness_ = std::make_shared<Texture>(path + "DefaultMaterial_Metallic.png");
        auto tex_ao_        = std::make_shared<Texture>(path + "DefaultMaterial_Mixed_AO.png");

        for(s32 y = 0; y < HEIGHT; ++y) {
            for(s32 x = 0; x < WIDTH; ++x) {
                auto model = std::make_shared<Model>("data/Sample/Sci-fi_Box/Sci-fi Box.mv1");

                // モデルに設定されているテクスチャを上書き
                model->overrideTexture(Model::TextureType::Diffuse, tex_albedo_);
                model->overrideTexture(Model::TextureType::Normal, tex_normal_);
                model->overrideTexture(Model::TextureType::Roughness, tex_roughness_);
                model->overrideTexture(Model::TextureType::Metalness, tex_metalness_);
                model->overrideTexture(Model::TextureType::AO, tex_ao_);

                model_boxes1_.emplace_back(std::move(model));
            }
        }
    }

    // コンテナボックス
    {
        std::string path = "data/Sample/Sci-fi_Container/Textures/";

        auto tex_albedo_    = std::make_shared<Texture>(path + "Sci-fi_Box_AlbedoTransparency.png");
        auto tex_normal_    = std::make_shared<Texture>(path + "Sci-fi_Box_Normal_DirectX.png");
        auto tex_roughness_ = std::make_shared<Texture>(path + "Sci-fi_Box_MetallicSmoothness.png");
        auto tex_metalness_ = std::make_shared<Texture>(path + "Sci-fi_Box_MetallicSmoothness.png");
        auto tex_ao_        = std::make_shared<Texture>(path + "Sci-fi_Box_AO.png");

        for(s32 y = 0; y < HEIGHT; ++y) {
            for(s32 x = 0; x < WIDTH; ++x) {
                auto model = std::make_shared<Model>("data/Sample/Sci-fi_Container/Sci-fi Container.mv1");

                // モデルに設定されているテクスチャを上書き
                model->overrideTexture(Model::TextureType::Diffuse, tex_albedo_);
                model->overrideTexture(Model::TextureType::Normal, tex_normal_);
                model->overrideTexture(Model::TextureType::Roughness, tex_roughness_);
                model->overrideTexture(Model::TextureType::Metalness, tex_metalness_);
                model->overrideTexture(Model::TextureType::AO, tex_ao_);

                model_boxes2_.emplace_back(std::move(model));
            }
        }
    }

    // ドラム缶
    {
        std::string path = "data/Sample/oil_barrels_pbr/textures/";

        auto tex_albedo_    = std::make_shared<Texture>(path + "drum2_base_color.png");
        auto tex_normal_    = std::make_shared<Texture>(path + "drum2_normal.png");
        auto tex_roughness_ = std::make_shared<Texture>(path + "drum2_roughness.png");
        auto tex_metalness_ = std::make_shared<Texture>(path + "drum2_metallic.png");
        auto tex_ao_        = std::make_shared<Texture>(path + "drum2_ambient.png");

        for(s32 y = 0; y < HEIGHT; ++y) {
            for(s32 x = 0; x < WIDTH; ++x) {
                auto model = std::make_shared<Model>("data/Sample/oil_barrels_pbr/barrel.mv1");

                // モデルに設定されているテクスチャを上書き
                model->overrideTexture(Model::TextureType::Diffuse, tex_albedo_);
                model->overrideTexture(Model::TextureType::Normal, tex_normal_);
                model->overrideTexture(Model::TextureType::Roughness, tex_roughness_);
                model->overrideTexture(Model::TextureType::Metalness, tex_metalness_);
                model->overrideTexture(Model::TextureType::AO, tex_ao_);

                model_barrel_.emplace_back(std::move(model));
            }
        }
    }

    // 三角コーン
    {
        std::string path = "data/Sample/Traffic_Cone/";

        auto tex_albedo_    = std::make_shared<Texture>(path + "Red_Dirty_Traffic Cone_Base_Color.png");
        auto tex_normal_    = std::make_shared<Texture>(path + "Dirty_Traffic Cone_Normal_DirectX.png");
        auto tex_roughness_ = std::make_shared<Texture>(path + "Dirty_Traffic Cone_Roughness.png");
        auto tex_metalness_ = std::make_shared<Texture>(path + "Dirty_Traffic Cone_Metallic.png");
        auto tex_ao_        = std::make_shared<Texture>(path + "Dirty_Traffic Cone_AO.png");

        for(s32 y = 0; y < HEIGHT; ++y) {
            for(s32 x = 0; x < WIDTH; ++x) {
                auto model = std::make_shared<Model>("data/Sample/Traffic_Cone/Traffic Cone.mv1");

                // モデルに設定されているテクスチャを上書き
                model->overrideTexture(Model::TextureType::Diffuse, tex_albedo_);
                model->overrideTexture(Model::TextureType::Normal, tex_normal_);
                model->overrideTexture(Model::TextureType::Roughness, tex_roughness_);
                model->overrideTexture(Model::TextureType::Metalness, tex_metalness_);
                model->overrideTexture(Model::TextureType::AO, tex_ao_);

                model_cone_.emplace_back(std::move(model));
            }
        }
    }

    //----------------------------------------------------------
    // 剛体を作成
    //----------------------------------------------------------
    for(s32 y = 0; y < HEIGHT; ++y) {
        for(s32 x = 0; x < WIDTH; ++x) {
            u32 type = rand() % 4;

            std::shared_ptr<physics::RigidBody> rigid_body;

            switch(type) {
            case 0:
                rigid_body = physics::createRigidBody(shape::Box{float3(0.5f, 0.5f, 0.5f)},
                                                      physics::ObjectLayers::MOVING);
                break;
            case 1:
                rigid_body = physics::createRigidBody(shape::Box{float3(0.5f, 0.5f, 0.5f)},
                                                      physics::ObjectLayers::MOVING);
                break;
            case 2:
                rigid_body = physics::createRigidBody(shape::Cylinder{0.5f, 0.375f}, physics::ObjectLayers::MOVING);
                break;
            case 3:
                rigid_body = physics::createRigidBody(shape::ConvexHull(model_cone_[0].get()),
                                                      physics::ObjectLayers::MOVING);
                break;
            default:
                break;
            }

            float3 position = float3((x - WIDTH / 2) * 1.5f, y * 1.5f + 2.0f, 0.0f);

            // やや位置をずらして崩れやすくする
            //position.x += rand() * 0.0001f;
            //position.z += rand() * 0.0001f;

            // 座標設定
            rigid_body->setPosition(position);

            // 跳ね返り係数
            rigid_body->setRestitution(0.5f);

            // 任意の値(自由な値で設定可能)
            rigid_body->setData(type);

            rigid_bodies_.emplace_back(std::move(rigid_body));
        }
    }

    //----------------------------------------------------------
    // 床を作成
    //----------------------------------------------------------
    body_floor_ = physics::createRigidBody(shape::Box{float3(63.0f, 1.0f, 63.0f)},
                                           physics::ObjectLayers::NON_MOVING,   // 静的グループ
                                           physics::MotionType::Static);        // 静的
    body_floor_->setPosition(float3(0.0f, -1.0f, 0.0f));

    // シミュレーションを最適化
    // (この処理は必ず呼ばなければならないわけではありません)
    physics::Engine::instance()->optimize();
}
