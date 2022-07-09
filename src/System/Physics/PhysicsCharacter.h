//---------------------------------------------------------------------------
//! @file   PhysicsCharacter.h
//! @brief  キャラクターコントローラー
//---------------------------------------------------------------------------
#pragma once

namespace shape
{
class Base;
}

namespace physics
{

//===========================================================================
//! キャラクターコントローラー
//===========================================================================
class Character
{
public:
    class ContactSettings;
    class ContactListener;

    // 接地状態
    enum class GroundState
    {
        OnGround,   ///< キャラクターが地上にいて自由に動ける状態
        Sliding,    ///< キャラクターが急すぎる斜面にいて滑り始めている
        InAir,      ///< キャラクターは空宙にいる
    };

    // コンストラクタ
    Character() = default;

    //----------------------------------------------------------
    //! @name   コンタクトリスナー
    //----------------------------------------------------------
    //@{

    //! コンタクトリスナーを設定
    virtual void setListener(physics::Character::ContactListener* listener) = 0;

    //! コンタクトリスナーを取得
    virtual physics::Character::ContactListener* listener() const = 0;

    //@}
    //----------------------------------------------------------
    //! @name   パラメーター
    //----------------------------------------------------------
    //@{

    //! 速度を取得 (単位:m/s)
    virtual float3 linearVelocity() const = 0;

    //! 速度を設定 (単位:m/s)
    virtual void setLinearVelocity(const float3& linear_velocity) = 0;

    //! 位置を取得
    virtual float3 position() const = 0;

    //! 位置を設定
    virtual void setPosition(const float3& p) = 0;

    //! 回転姿勢を取得
    virtual quaternion rotation() const = 0;

    //! 回転姿勢を設定
    virtual void setRotation(const quaternion& r) = 0;

    //! ワールド行列を取得
    virtual matrix worldMatrix() const = 0;

    //! 重心の変換行列を取得
    virtual matrix centerOfMassTransform() const = 0;

    //! 質量を設定 (単位:kg)
    virtual void setMass(f32 mass) = 0;

    //! キャラクターがまだ歩ける最大傾斜角度を設定する (単位:ラジアン)
    virtual void setMaxSlopeAngle(f32 max_slope_angle) = 0;

    //! キャラクターが他の物体を押すことができる最大の力を設定（単位:N）
    virtual void setMaxStrength(f32 max_strength) = 0;

    //! ジオメトリからどれだけ離れられるかの距離を取得
    virtual f32 characterPadding() const = 0;

    //@}

    //! 更新
    //! 現在の速度に応じてキャラクターを移動させます。
    //! @param [in] delta_time  シミュレーションステップ時間
    virtual void update(f32 delta_time) = 0;

    //! 移動
    //! @param  [in]    delta_time      シミュレーションステップ時間
    //! @param  [in]    move_vector     移動ベクトル
    //! @param  [in]    old_position    移動前の位置( update()実行前の座標が必要です )
    //! @param  [in]    jump_vector     ジャンプ方向ベクトル(ジャンプしない場合は0で渡します)
    //! @return 新しい補正後の移動速度
    //! 内部でwalkStairs()による階段歩行も行います
    virtual float3 move(f32           delta_time,
                        const float3& move_vector,
                        const float3& old_position,
                        const float3& jump_vector = float3(0.0f, 0.0f, 0.0f)) = 0;

    //! 階段を歩けるかどうかを取得
    //! @details この関数はキャラクターが急すぎる斜面（垂直な壁など）に移動した場合にtrueを返します。
    //! 階段を上ろうとする場合はwalkStairsを呼び出すことになります。
    virtual bool canWalkStairs() const = 0;

    //! 階段歩行
    //! @param  [in]    delta_time          シミュレーションのステップ時間
    //! @param  [in]    step_up             ステップアップする方向と距離（これが最大ステップ高に相当する）
    //! @param  [in]    step_forward        踏み台昇降の後に前に踏み出す方向と距離
    //! @param  [in]    step_forward_test   床を検査する方向と距離
    //! @param inStepDownExtra  ステップダウンする最後に加算される平行移動。上より下にステップダウンできるようにします。不要な場合は0に設定します。
    //! @return 階段の歩行が成功した場合はtrueが戻ります
    //!
    //! 引数 step_forward_test について
    //! 高い周波数で走行している場合 step_forward は非常に小さくなり、降りるときに階段の側面にぶつかる可能性があります。
    //! そのため、最大傾斜角度に違反する法線が発生する可能性があります。
    //! このような場合は上昇位置からこの距離を使って再度テストし、有効なスロープを見つけるかどうかを確認します。
    virtual bool walkStairs(f32           delta_time,
                            const float3& step_up,
                            const float3& step_forward,
                            const float3& step_forward_test,
                            const float3& step_down_extra = float3(0.0f, 0.0f, 0.0f)) = 0;

    //! 衝突点を再検出
    //! この機能はキャラクターがテレポートした後に、新しい世界とのコンタクトを決定するために使用することができます。
    virtual void refleshContacts() = 0;

    //! キャラクターの形状を入れ替える（しゃがみ、構え方など）
    //! @param  [in]    shape           形状
    //! @param  [in]    shape_offset    形状位置をずらすオフセット
    //! @retval true    正常終了(形状の入れ替え成功)
    //! @retval false   形状の入れ替え後に衝突物があり、切り替えできなかった場合
    virtual bool setShape(const shape::Base& shape, const float3& shape_offset = float3(0.0f, 0.0f, 0.0f)) = 0;

    //! シェイプを取得
    virtual shape::Base* shape() const = 0;

    //! シェイプの移動オフセットを取得
    virtual float3 shapeOffset() const = 0;

    //----------------------------------------------------------
    //! @name   接地判定関連
    //----------------------------------------------------------
    //@{

    //! 現在の接地状態を取得
    virtual Character::GroundState groundState() const = 0;

    //! 接地している足元の位置座標を取得
    virtual float3 groundPosition() const = 0;

    //! 接地している足元の法線を取得
    virtual float3 groundNormal() const = 0;

    //! 接地している足元の速度を取得
    virtual float3 groundVelocity() const = 0;

    //! キャラクターが立っているオブジェクトのボディIDを取得
    virtual u64 groundBodyID() const = 0;

    //@}
};

//===========================================================================
//! 衝突設定情報
//! キャラクタの衝突応答の動作をオーバーライドするための設定です
//===========================================================================
class Character::ContactSettings
{
public:
    bool can_push_character_ = true;   //!< 他のボディがキャラクターを押すことができるかどうか
    bool can_receive_impulses_ = true;   //!< キャラクタがー他のボディから押されるかどうか
};

//===========================================================================
//! キャラクターコンタクトリスナー
//! キャラクターに登録することでイベントをコールバックで受け取ることができます
//===========================================================================
class Character::ContactListener
{
public:
    //! デストラクタ
    virtual ~ContactListener() = default;

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
        result.can_push_character_ = true;   // 相手のオブジェクトから押される
        result.can_receive_impulses_ = true;   // 相手のオブジェクトをキャラクターで押すことができる
    }
};

//===========================================================================
//! @name   生成
//===========================================================================
//@{

// キャラクターを作成
//! @param  [in]    layer   オブジェクトレイヤー(カテゴリー)
std::shared_ptr<physics::Character> createCharacter(u16 layer);

//@}

}   // namespace physics
