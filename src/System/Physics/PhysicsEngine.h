//---------------------------------------------------------------------------
//! @file   PhysicsEngine.h
//! @brief  物理シミュレーション
//---------------------------------------------------------------------------
#pragma once

class Ray;

namespace JPH
{
class Vec3;
class Quat;
class Mat44;

class PhysicsSystem;
class BodyInterface;
class TempAllocator;
}   // namespace JPH

namespace physics
{

enum ObjectLayers : u16;

//--------------------------------------------------------------
//! レイキャストの結果
//--------------------------------------------------------------
struct RayCastResult
{
    u64 body_id_;   // ボディID
    f32 t_;         // 衝突点のパラメーターt  hit_position = start + t * (end - start)
};

//===========================================================================
// 物理シミュレーション
//===========================================================================
class Engine
{
protected:
    //! デフォルトコンストラクタ
    Engine() = default;

public:
    //! デストラクタ
    virtual ~Engine() = default;

    //! 更新
    //! @param  [in]    dt  経過時間⊿t
    virtual void update(f32 dt) = 0;

    //  シーンにレイをキャスト
    //! @param  [in]    ray     シーンに飛ばすレイ
    //! @param  [out]   result  衝突結果
    //! @retval true    衝突あり。resultに結果が格納されています
    //! @retval false   衝突なし。結果は無効
    virtual bool castRay(const Ray& ray, RayCastResult& result) = 0;

    //! シーン最適化を実行
    //! @details これによって衝突検出のパフォーマンスが向上します
    //! @attention かなり重い処理のため、毎フレームまたは新しいレベルセクションのストリーミング時などには
    //! @attention 絶対に呼び出さない方がよいでしょう。
    virtual void optimize() = 0;

    //! オブジェクトレイヤーをカスタム設定
    //! @param  [in]    layers      自前レイヤーとphysics::ObjectLayersが対応するペアの配列先頭(nullptrで解除)
    //! @param  [in]    layerCount  配列数
    virtual void setLayerAlias(const std::pair<physics::ObjectLayers, u16>* layers, size_t layerCount) = 0;

    //! オブジェクトレイヤー同士が衝突するかどうかをカスタマイズ
    //! コールバック関数の [第1引数] 対象のレイヤー [第2引数] 相手のレイヤー
    //! [戻り値] true:衝突する false:衝突しない
    virtual void overrideLayerCollide(std::function<bool(u16, u16)> callback) = 0;

    //----------------------------------------------------------
    //! @name   参照
    //----------------------------------------------------------
    //@{

    //! 重力を取得
    virtual float3 gravity() const = 0;

    //! 正常に初期化されているかどうかを取得
    virtual bool isValid() const = 0;

    //! Physicsインスタンスを取得
    static Engine* instance();

    //@}
    //----------------------------------------------------------
    //! @name   JoltPhysics用のネイティブ情報取得インターフェイス
    //----------------------------------------------------------
    //@{

    //! [JPH] Physicsシステムを取得
    static JPH::PhysicsSystem* physicsSystem();

    //! [JPH] ボディインターフェイスを取得
    static JPH::BodyInterface* bodyInterface();

    //! [JPH] テンポラリアロケーターを取得
    static JPH::TempAllocator* tempAllocator();

    //@}
};

//===========================================================================
//! @name   生成
//===========================================================================
//@{

//  物理シミュレーションクラスを作成
std::unique_ptr<physics::Engine> createPhysics();

//@}

JPH::Vec3  castJPH(const float3& v);
float3     castJPH(const JPH::Vec3& v);
JPH::Quat  castJPH(const quaternion& q);
quaternion castJPH(const JPH::Quat& v);
matrix     castJPH(const JPH::Mat44& mat44);

}   // namespace physics
