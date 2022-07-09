//---------------------------------------------------------------------------
//! @file   PhysicsEngine.cpp
//! @brief  物理シミュレーション
//---------------------------------------------------------------------------
#include "PhysicsEngine.h"
#include "PhysicsLayer.h"
#include "System/Geometry.h"

//--------------------------------------------------------------
// JoltPhysicsインクルードファイル
//--------------------------------------------------------------
#include <Jolt/Jolt.h>

#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Collision/Shape/HeightFieldShape.h>
#include <Jolt/Physics/Collision/Shape/MeshShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/ConvexHullShape.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/TaperedCapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/CylinderShape.h>
#include <Jolt/Physics/Collision/Shape/TriangleShape.h>
#include <Jolt/Physics/Collision/Shape/StaticCompoundShape.h>
#include <Jolt/Physics/Collision/Shape/MutableCompoundShape.h>
#include <Jolt/Physics/Collision/Shape/ScaledShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>

#include <Jolt/Physics/Collision/RayCast.h>
#include <Jolt/Physics/Collision/CastResult.h>
#include <Jolt/Physics/Collision/CollisionCollectorImpl.h>
#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseQuery.h>

#include <iostream>
#include <sstream>
#include <string>

namespace
{

//===========================================================================
//! @name   コールバックのカスタマイズ
//===========================================================================
//@{

//---------------------------------------------------------------------------
//! トレース用のコールバック
//---------------------------------------------------------------------------
void TraceImpl(const char* format, ...)
{
    va_list list;
    va_start(list, format);
    char buffer[1024];
    vsnprintf_s(buffer, sizeof(buffer), format, list);

    //----------------------------------------------------------
    // 「出力」ウィンドウに出力
    //----------------------------------------------------------
    OutputDebugStringA(buffer);
}

#ifdef JPH_ENABLE_ASSERTS

//---------------------------------------------------------------------------
//! assert用のコールバック
//---------------------------------------------------------------------------
bool AssertFailedImpl(const char* expression, const char* message, const char* file, JPH::uint line)
{
    std::ostringstream oss{};
    oss << file                          // ファイル名
        << ":" << line                   // 行
        << ": (" << expression << ") "   // 内容
        << (message ? message : "")      // メッセージ
        << std::endl;

    //----------------------------------------------------------
    // 「出力」ウィンドウに出力
    //----------------------------------------------------------
    std::string s = oss.str();
    OutputDebugStringA(s.c_str());

    return true;
};

#endif   // JPH_ENABLE_ASSERTS

//@}
//===========================================================================
//! @name   Broad-phase レイヤーのカスタマイズ
//===========================================================================
//@{

// Broad-phaseの各レイヤーはBroad-phaseの個別のバウンディングボリュームツリーになります。
// 少なくとも、動かないオブジェクトと動くオブジェクトのレイヤーを用意して
// フレームごとに静的オブジェクトでいっぱいのツリー更新を実行する必要がないようにする必要があります。
//
// オブジェクトレイヤーとBroad-phase Layerとの間に1対1のマッピングが可能ですが、
// 多くのオブジェクトレイヤーがある場合は多くのブロードフェーズツリーを作成することになり、効率的ではありません。
// Broad-phase Layerを微調整したい場合 JPH_TRACK_BROADPHASE_STATS を定義して出力メッセージで報告される統計情報を見てください。
namespace BroadPhaseLayers
{

constexpr JPH::BroadPhaseLayer NON_MOVING(0);
constexpr JPH::BroadPhaseLayer MOVING(1);
constexpr JPH::BroadPhaseLayer DEBRIS(2);
constexpr JPH::BroadPhaseLayer SENSOR(3);
constexpr JPH::uint            NUM_LAYERS(4);

//! オブジェクトからBroad-phaseレイヤーへの変換テーブル
std::vector<JPH::BroadPhaseLayer> object_to_broad_phase_;

//! オブジェクトレイヤー同士が衝突するかどうかをカスタマイズするコールバック関数
std::function<bool(u16, u16)> can_object_layer_collide_;

}   // namespace BroadPhaseLayers

//@}
//===========================================================================
//! @name   レイヤーのカスタマイズ
//===========================================================================
//@{

// 2つのオブジェクトレイヤーが衝突可能かどうかを判断するカスタム関数
bool MyObjectCanCollide(JPH::ObjectLayer object1, JPH::ObjectLayer object2)
{
    // カスタム関数がある場合は実行
    if(BroadPhaseLayers::can_object_layer_collide_) {
        return BroadPhaseLayers::can_object_layer_collide_(object1, object2);
    }

    // デフォルトの設定
    switch(object1) {
    case physics::ObjectLayers::NON_MOVING:
        return object2 == physics::ObjectLayers::MOVING ||   //
               object2 == physics::ObjectLayers::DEBRIS;     //
    case physics::ObjectLayers::MOVING:
        return object2 == physics::ObjectLayers::NON_MOVING ||   //
               object2 == physics::ObjectLayers::MOVING ||       //
               object2 == physics::ObjectLayers::SENSOR;         //
    case physics::ObjectLayers::DEBRIS:
        return object2 == physics::ObjectLayers::NON_MOVING;
    case physics::ObjectLayers::SENSOR:
        return object2 == physics::ObjectLayers::MOVING;
    default:
        JPH_ASSERT(false);
        return false;
    }
};

//@}
//===========================================================================
//! BroadPhaseLayerInterface 実装部
//! @details オブジェクト層とBroad-phase層の間のマッピングを定義します
//===========================================================================
class BPLayerInterfaceImpl final : public JPH::BroadPhaseLayerInterface
{
public:
    BPLayerInterfaceImpl() {}

    //! Broad-phaseレイヤーの個数を取得
    virtual JPH::uint GetNumBroadPhaseLayers() const override { return BroadPhaseLayers::NUM_LAYERS; }

    //! Broad-phaseレイヤーを取得
    virtual JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer layer) const override
    {
        return BroadPhaseLayers::object_to_broad_phase_[layer];
    }

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)

    //! Broad-phaseレイヤーの名前を取得
    virtual const char* GetBroadPhaseLayerName(JPH::BroadPhaseLayer layer) const override
    {
        switch(static_cast<JPH::BroadPhaseLayer::Type>(layer)) {
        case BroadPhaseLayers::NON_MOVING:
            return "NON_MOVING";
        case BroadPhaseLayers::MOVING:
            return "MOVING";
        case BroadPhaseLayers::DEBRIS:
            return "DEBRIS";
        case BroadPhaseLayers::SENSOR:
            return "SENSOR";
        case BroadPhaseLayers::UNUSED:
            return "UNUSED";
        default:
            JPH_ASSERT(false);
            return "invalid";
        }
    }

#endif   // JPH_EXTERNAL_PROFILE || JPH_PROFILE_ENABLED

private:
};

//---------------------------------------------------------------------------
//! Broad-phase層同士が衝突するかどうかを判定するカスタマイズ
//---------------------------------------------------------------------------
bool MyBroadPhaseCanCollide(JPH::ObjectLayer layer1, JPH::BroadPhaseLayer layer2)
{
    auto layer1_broad_phase = BroadPhaseLayers::object_to_broad_phase_[layer1];

    if(layer1_broad_phase == BroadPhaseLayers::NON_MOVING) {
        return layer2 == BroadPhaseLayers::MOVING;
    }
    if(layer1_broad_phase == BroadPhaseLayers::MOVING) {
        return layer2 == BroadPhaseLayers::NON_MOVING ||   //
               layer2 == BroadPhaseLayers::MOVING ||       //
               layer2 == BroadPhaseLayers::SENSOR;         //
    }
    if(layer1_broad_phase == BroadPhaseLayers::DEBRIS) {
        return layer2 == BroadPhaseLayers::NON_MOVING;
    }
    if(layer1_broad_phase == BroadPhaseLayers::SENSOR) {
        return layer2 == BroadPhaseLayers::MOVING;
    }

    JPH_ASSERT(false);
    return false;
}

//===========================================================================
//! コンタクトListenerの実装サンプル
//! @see JPH::ContactListener
//===========================================================================
class MyContactListener : public JPH::ContactListener
{
public:
    //! コンタクトを有効にするかどうかの判定
    virtual JPH::ValidateResult
    OnContactValidate([[maybe_unused]] const JPH::Body&               body1,
                      [[maybe_unused]] const JPH::Body&               body2,
                      [[maybe_unused]] const JPH::CollideShapeResult& collision_result) override
    {
        // std::cout << "コールバックが有効か確認." << std::endl;

        // 作成前のコンタクトを無視できるようにできる
        // （但し、レイヤーを使ってオブジェクトを衝突させないようにするほうが軽い）
        return JPH::ValidateResult::AcceptAllContactsForThisBodyPair;
    }

    //! コンタクトが追加される時
    virtual void OnContactAdded([[maybe_unused]] const JPH::Body&            body1,
                                [[maybe_unused]] const JPH::Body&            body2,
                                [[maybe_unused]] const JPH::ContactManifold& manifold,
                                [[maybe_unused]] JPH::ContactSettings&       settings) override
    {
        // std::cout << "コンタクトが追加されました." << std::endl;
    }

    //! コンタクトが継続している時
    virtual void OnContactPersisted([[maybe_unused]] const JPH::Body&            body1,
                                    [[maybe_unused]] const JPH::Body&            body2,
                                    [[maybe_unused]] const JPH::ContactManifold& manifold,
                                    [[maybe_unused]] JPH::ContactSettings&       settings) override
    {
        // std::cout << "コンタクトが継続." << std::endl;
    }

    //! コンタクトが削除される時
    virtual void OnContactRemoved([[maybe_unused]] const JPH::SubShapeIDPair& sub_shape_pair) override
    {
        // std::cout << "コンタクトが削除されました." << std::endl;
    }
};

//===========================================================================
//! Activation Listenerの実装サンプル
//! @see JPH::BodyActivationListener
//===========================================================================
class MyBodyActivationListener : public JPH::BodyActivationListener
{
public:
    //! アクティブになる時
    virtual void OnBodyActivated([[maybe_unused]] const JPH::BodyID& body_id,
                                 [[maybe_unused]] JPH::uint64        body_user_data) override
    {
        // std::cout << "オブジェクトがアクティブになりました." << std::endl;
    }

    //! 非アクティブになる時
    virtual void OnBodyDeactivated([[maybe_unused]] const JPH::BodyID& body_id,
                                   [[maybe_unused]] JPH::uint64        body_user_data) override
    {
        // std::cout << "オブジェクトが非アクティブ(スリープ状態)に移行しました." << std::endl;
    }
};

}   // namespace

namespace physics
{

//===========================================================================
//! 物理シミュレーション(実装部)
//===========================================================================
class EngineImpl final : public physics::Engine
{
public:
    // デフォルトコンストラクタ
    EngineImpl();

    // デストラクタ
    virtual ~EngineImpl();

    //  更新
    //! @param  [in]    dt  経過時間⊿t
    virtual void update(f32 dt) override;

    //  シーンにレイをキャスト
    //! @param  [in]    ray     シーンに飛ばすレイ
    //! @param  [out]   result  衝突結果
    //! @retval true    衝突あり。resultに結果が格納されています
    //! @retval false   衝突なし。結果は無効
    virtual bool castRay(const Ray& ray, RayCastResult& result) override;

    //  解放
    void clear();

    //  シーン最適化を実行
    //! @details これによって衝突検出のパフォーマンスが向上します
    //! @attention かなり重い処理のため、毎フレームまたは新しいレベルセクションのストリーミング時などには
    //! @attention 絶対に呼び出さない方がよいでしょう。
    virtual void optimize() override;

    //  オブジェクトレイヤーをカスタム設定
    //! @param  [in]    layers      自前レイヤーとphysics::ObjectLayersが対応するペアの配列先頭(nullptrで解除)
    //! @param  [in]    layerCount  配列数
    virtual void setLayerAlias(const std::pair<physics::ObjectLayers, u16>* layers, size_t layerCount) override;

    //  オブジェクトレイヤー同士が衝突するかどうかをカスタマイズ
    //! コールバック関数の [第1引数] 対象のレイヤー [第2引数] 相手のレイヤー
    //! [戻り値] true:衝突する false:衝突しない
    virtual void overrideLayerCollide(std::function<bool(u16, u16)> callback) override;

    // 重力を取得
    virtual float3 gravity() const override;

    //  正常に初期化されているかどうかを取得
    virtual bool isValid() const override;

    //  [JPH] Physicsシステムを取得
    static JPH::PhysicsSystem* physicsSystem();

    //  [JPH] ボディインターフェイスを取得
    static JPH::BodyInterface* bodyInterface();

    //  [JPH] テンポラリアロケーターを取得
    static JPH::TempAllocator* tempAllocator();

    //----------------------------------------------------------
    //! @name   copy/move禁止
    //----------------------------------------------------------
    //@{

private:
    EngineImpl(const EngineImpl&)     = delete;
    EngineImpl(EngineImpl&&)          = delete;
    void operator=(const EngineImpl&) = delete;
    void operator=(EngineImpl&&)      = delete;

    //@}

private:
    std::unique_ptr<JPH::Factory>             jph_factory_;          //!< Factoryクラス
    std::unique_ptr<JPH::JobSystemThreadPool> job_system_;           //!< ジョブシステム
    std::unique_ptr<JPH::PhysicsSystem>       jph_physics_system_;   //!< Physicsシステム
    static inline JPH::PhysicsSystem* physics_system_ = nullptr;   //!< Physicsシステムのstaticアクセス用の参照
    static inline JPH::BodyInterface* body_interface_ = nullptr;   //!< ボディインターフェイス参照
    static inline JPH::TempAllocator* temp_allocator_ = nullptr;   //!< ! テンポラリアロケーターのstaticアクセス用の参照

    //! テンポラリアロケーター
    //! @details 物理演算の更新中にアロケーションを行う必要がないように、事前アロケーションを行っています。
    //!          もし事前割り当てをしたくない場合はTempAllocatorMallocを使って malloc/freeにフォールバックすることもできます。
    JPH::TempAllocatorImpl jph_temp_allocator_ = JPH::TempAllocatorImpl(64 * 1024 * 1024);

    //! オブジェクト層からBroad-phase層へのマッピングテーブル
    //! @attention これはインターフェースですのでPhysicsシステムはこのインスタンスへ参照します。
    //!            このインスタンスはシステムの解放まで必要です。
    BPLayerInterfaceImpl broad_phase_layer_interface_;

    MyBodyActivationListener body_activation_listener_;   //!< ユーザーコールバック BodyActivationListener
    MyContactListener        contact_listener_;           //!< ユーザーコールバック ContactListener

    bool is_valid_ = false;   //!< 正常に初期化されているか
};

namespace
{

physics::EngineImpl* physics_ = nullptr;   //!< Physicsインスタンス

}

//---------------------------------------------------------------------------
//! デフォルトコンストラクタ
//---------------------------------------------------------------------------
EngineImpl::EngineImpl()
{
    physics_        = this;
    temp_allocator_ = &jph_temp_allocator_;

    //----------------------------------------------------------
    // トレースコールバックを登録
    //----------------------------------------------------------
    JPH::Trace = TraceImpl;
    JPH_IF_ENABLE_ASSERTS(JPH::AssertFailed = AssertFailedImpl;)

    //----------------------------------------------------------
    // Factoryを作成
    //----------------------------------------------------------
    jph_factory_ = std::make_unique<JPH::Factory>();

    // JoltPhysics内部変数に登録する
    JPH::Factory::sInstance = jph_factory_.get();

    //----------------------------------------------------------
    // すべてのJoltPhysicsの型をFactoryに登録
    //----------------------------------------------------------
    JPH::RegisterTypes();

    // 物理ジョブを複数スレッドで実行するジョブシステムが必要です。
    // JoltPhysicsは基本的には自前のジョブスケジューラの上で実行することが出来ます。
    // このJobSystemThreadPoolは実装例です。
    job_system_ = std::make_unique<JPH::JobSystemThreadPool>(JPH::cMaxPhysicsJobs,
                                                             JPH::cMaxPhysicsBarriers,
                                                             std::thread::hardware_concurrency() - 1);

    //----------------------------------------------------------
    // Physicsシステムを初期化
    //----------------------------------------------------------
    {
        // Physicsシステムに追加できる剛体の最大量
        // この個数以上追加しようとするとエラーが発生します。
        constexpr JPH::uint MAX_BODIES = 65536;

        // 剛体を同時アクセスから保護するために割り当てるべきミューテックスの数です。(default:0)
        constexpr JPH::uint BODY_MUTEX_COUNT = 0;

        // いつでもキューに入れられるボディペアの最大数
        // Broad-phaseではバウンディングボックスに基づいて重複するボディペアを検出してNarrow-phaseのキューに挿入します
        // このバッファを小さくしすぎると、キューが一杯になりBroad-phaseのジョブがNarrow-phaseの実行をし始めることになります。
        // これは若干効率的ではありません。
        constexpr JPH::uint MAX_BODY_PAIRS = 65536;

        // コンタクト拘束バッファの最大サイズ
        // この数よりも多くの接触（ボディ間の衝突）が検出された場合、これらの接触は無視されボディはワールド突き抜けて落下し始めます。
        constexpr JPH::uint MAX_CONTACT_CONSTRAINTS = 65536;

        // Physicsシステムを初期化作成
        jph_physics_system_ = std::make_unique<JPH::PhysicsSystem>();

        jph_physics_system_->Init(MAX_BODIES,
                                  BODY_MUTEX_COUNT,
                                  MAX_BODY_PAIRS,
                                  MAX_CONTACT_CONSTRAINTS,
                                  broad_phase_layer_interface_,
                                  MyBroadPhaseCanCollide,
                                  MyObjectCanCollide);

        // パラメーター設定
        JPH::PhysicsSettings settings{};

        jph_physics_system_->SetPhysicsSettings(settings);

        // 重力を設定
        jph_physics_system_->SetGravity(JPH::Vec3(0.0f, -9.81f, 0.0f));

        // ポインタを保存
        physics_system_ = jph_physics_system_.get();
    }

    //----------------------------------------------------------
    // ボディインターフェイスを保存
    //----------------------------------------------------------
    {
        constexpr bool thread_safe = true;

        if constexpr(thread_safe) {
            body_interface_ = &jph_physics_system_->GetBodyInterface();   // Lockあり
        }
        else {
            body_interface_ = &jph_physics_system_->GetBodyInterfaceNoLock();   // Lockなし(利用には注意)
        }
    }

    // ボディがアクティブになりスリープ状態になると、BodyActivationListenerが通知されます。
    // これはジョブから呼び出されるので、ここで行うことはスレッドセーフである必要があることに注意してください。
    // (登録は完全に任意です)
    jph_physics_system_->SetBodyActivationListener(&body_activation_listener_);

    // ボディが衝突する（しそうな）ときと再び離れるときに、ContactListenerに通知されます。
    // これはジョブから呼び出されるので、ここで何をするにしてもスレッドセーフである必要があることに注意してください。
    // (登録は完全に任意です)
    jph_physics_system_->SetContactListener(&contact_listener_);

    // デフォルトのBroadPhase変換テーブルを初期化
    setLayerAlias(nullptr, 0);

    is_valid_ = true;
}

//---------------------------------------------------------------------------
//! デストラクタ
//---------------------------------------------------------------------------
EngineImpl::~EngineImpl()
{
    clear();

    physics_ = nullptr;
}

//---------------------------------------------------------------------------
//! 更新
//---------------------------------------------------------------------------
void EngineImpl::update(f32 dt)
{
    // 1/60秒より大きなステップを取る場合、シミュレーションを安定させるために
    // 複数回の衝突ステップを行う必要があります。1/60秒ごとに1回の衝突ステップを実行します（切り上げ）
    const u32 collision_steps = 1;

    // より正確なステップ結果を得たい場合は、コリジョンステップの中で複数のサブステップを行うことができます。
    const u32 integration_sub_steps = 1;   // 通常は1に設定します。

    // ワールドを時間経過させて更新
    jph_physics_system_->Update(dt, collision_steps, integration_sub_steps, temp_allocator_, job_system_.get());
}

//---------------------------------------------------------------------------
//! シーンにレイをキャスト
//---------------------------------------------------------------------------
bool EngineImpl::castRay(const Ray& ray, RayCastResult& result)
{
    JPH::RayCast ray_cast{castJPH(ray.position_), castJPH(ray.dir_)};

    auto& jph_narrow_phase = physics::Engine::physicsSystem()->GetNarrowPhaseQuery();

    JPH::RayCastResult jph_result;
    if(jph_narrow_phase.CastRay(ray_cast, jph_result)) {
        result.t_       = jph_result.mFraction;
        result.body_id_ = jph_result.mBodyID.GetIndexAndSequenceNumber();
        return true;
    }
    return false;
}

//---------------------------------------------------------------------------
//! 解放
//---------------------------------------------------------------------------
void EngineImpl::clear()
{
    // JoltPhysics内部変数のFactoryを登録解除
    // 実体の解放はスマートポインタが解放します
    JPH::Factory::sInstance = nullptr;
}

//---------------------------------------------------------------------------
//! シーン最適化を実行
//---------------------------------------------------------------------------
void EngineImpl::optimize()
{
    // 物理シミュレーションを開始する前にBroad-phaseを最適化することができます。
    // これによって衝突検出のパフォーマンスが向上します
    // これは高価な操作なので、毎フレームまたは新しいレベルセクションのストリーミング時などには絶対に呼び出さない方がよいでしょう。
    // 代替策はすべての新しいオブジェクトを一度に 1つずつではなく、バッチで挿入してBroad-phaseを効率的に維持することです。
    jph_physics_system_->OptimizeBroadPhase();   // ※このタイミングではまだオブジェクトが1つもないため無意味です。
}

//---------------------------------------------------------------------------
//! オブジェクトレイヤーをカスタム設定
//---------------------------------------------------------------------------
void EngineImpl::setLayerAlias(const std::pair<physics::ObjectLayers, u16>* layers, size_t layerCount)
{
    std::array<JPH::BroadPhaseLayer, 4> table{
        BroadPhaseLayers::NON_MOVING,
        BroadPhaseLayers::MOVING,
        BroadPhaseLayers::DEBRIS,
        BroadPhaseLayers::SENSOR,
    };

    //----------------------------------------------------------
    // オブジェクトからBroad-phaseレイヤーへの変換テーブル作成
    //----------------------------------------------------------
    if(layers) {
        // 値の最大値を検索して変換テーブルを作成
        // 都度検索すると負荷が高くなるため一発変換できる構造に変更しておく
        auto* myLayerMax = std::max_element(layers, layers + layerCount, [](const auto& l, const auto& r) {
            return l.second < r.second;
        });

        std::vector<physics::ObjectLayers>
            toObjectLayers_;   // カスタム設定したレイヤー番号を内部番号に変換するテーブル

        // 変換テーブルを最大数で設定
        BroadPhaseLayers::object_to_broad_phase_.resize(static_cast<size_t>(myLayerMax->second) + 1);

        for(size_t i = 0; i < layerCount; ++i) {
            BroadPhaseLayers::object_to_broad_phase_[layers[i].second] = table[layers[i].first];
        }
    }
    else {
        // デフォルト設定
        BroadPhaseLayers::object_to_broad_phase_.resize(physics::ObjectLayers::MAX_COUNT);
        BroadPhaseLayers::object_to_broad_phase_[physics::ObjectLayers::NON_MOVING] = BroadPhaseLayers::NON_MOVING;
        BroadPhaseLayers::object_to_broad_phase_[physics::ObjectLayers::MOVING]     = BroadPhaseLayers::MOVING;
        BroadPhaseLayers::object_to_broad_phase_[physics::ObjectLayers::DEBRIS]     = BroadPhaseLayers::DEBRIS;
        BroadPhaseLayers::object_to_broad_phase_[physics::ObjectLayers::SENSOR]     = BroadPhaseLayers::SENSOR;
    }
}

//---------------------------------------------------------------------------
//! オブジェクトレイヤー同士が衝突するかどうかをカスタマイズ
//---------------------------------------------------------------------------
void EngineImpl::overrideLayerCollide(std::function<bool(u16, u16)> callback)
{
    BroadPhaseLayers::can_object_layer_collide_ = callback;
}

//---------------------------------------------------------------------------
//! 重力を取得
//---------------------------------------------------------------------------
float3 EngineImpl::gravity() const
{
    return castJPH(jph_physics_system_->GetGravity());
}

//---------------------------------------------------------------------------
//! 正常に初期化されているかどうかを取得
//---------------------------------------------------------------------------
bool EngineImpl::isValid() const
{
    return is_valid_;
}

//---------------------------------------------------------------------------
//! [JPH] Physicsシステムを取得
//---------------------------------------------------------------------------
JPH::PhysicsSystem* EngineImpl::physicsSystem()
{
    return physics_system_;
}

//---------------------------------------------------------------------------
//! [JPH] ボディインターフェイスを取得
//---------------------------------------------------------------------------
JPH::BodyInterface* EngineImpl::bodyInterface()
{
    return body_interface_;
}

//---------------------------------------------------------------------------
//!  [JPH] テンポラリアロケーターを取得
//---------------------------------------------------------------------------
JPH::TempAllocator* EngineImpl::tempAllocator()
{
    return temp_allocator_;
}

//===========================================================================
// physics::Engine
//===========================================================================

//---------------------------------------------------------------------------
//!  Physicsインスタンスを取得
//---------------------------------------------------------------------------
physics::Engine* Engine::instance()
{
    return physics_;
}

//---------------------------------------------------------------------------
//! [JPH] Physicsシステムを取得
//---------------------------------------------------------------------------
JPH::PhysicsSystem* Engine::physicsSystem()
{
    return EngineImpl::physicsSystem();
}

//---------------------------------------------------------------------------
//! [JPH] ボディインターフェイスを取得
//---------------------------------------------------------------------------
JPH::BodyInterface* Engine::bodyInterface()
{
    return EngineImpl::bodyInterface();
}

//---------------------------------------------------------------------------
//!  [JPH] テンポラリアロケーターを取得
//---------------------------------------------------------------------------
JPH::TempAllocator* Engine::tempAllocator()
{
    return EngineImpl::tempAllocator();
}

//---------------------------------------------------------------------------
//! 物理シミュレーションクラスを作成
//---------------------------------------------------------------------------
std::unique_ptr<physics::Engine> createPhysics()
{
    return std::make_unique<EngineImpl>();
}

//===========================================================================
// キャスト
//===========================================================================

//---------------------------------------------------------------------------
//! float3 → JPH::Vec3 へキャスト
//---------------------------------------------------------------------------
JPH::Vec3 castJPH(const float3& v)
{
    return JPH::Vec3{v.x, v.y, v.z};
}

//---------------------------------------------------------------------------
//! JPH::Vec3 → float3 へキャスト
//---------------------------------------------------------------------------
float3 castJPH(const JPH::Vec3& v)
{
    return float3(v.GetX(), v.GetY(), v.GetZ());
}

//---------------------------------------------------------------------------
//! quaternion → JPH::Quat へキャスト
//---------------------------------------------------------------------------
JPH::Quat castJPH(const quaternion& q)
{
    return JPH::Quat{q.x, q.y, q.z, q.w};
}

//---------------------------------------------------------------------------
//! JPH::Quat → quaternion へキャスト
//---------------------------------------------------------------------------
quaternion castJPH(const JPH::Quat& v)
{
    return quaternion(v.GetX(), v.GetY(), v.GetZ(), v.GetW());
}

//---------------------------------------------------------------------------
//! JPH::Mat44 → matrix へキャスト
//---------------------------------------------------------------------------
matrix castJPH(const JPH::Mat44& mat44)
{
    auto c0 = mat44.GetColumn4(0);
    auto c1 = mat44.GetColumn4(1);
    auto c2 = mat44.GetColumn4(2);
    auto c3 = mat44.GetColumn4(3);

    float4 v[4];
    load(v[0], c0.mF32);
    load(v[1], c1.mF32);
    load(v[2], c2.mF32);
    load(v[3], c3.mF32);

    return matrix(v[0], v[1], v[2], v[3]);
}

}   // namespace physics
