//---------------------------------------------------------------------------
//! @file   TypeInfo.h
//! @brief  クラス型情報
//---------------------------------------------------------------------------
#pragma once

//===========================================================================
//! 型情報基底
//===========================================================================
class Type
{
public:
    //  コンストラクタ
    //! @param  [in]    class_name  クラスの名前
    //! @param  [in]    class_size  クラスのサイズ(単位:bytes)
    //! @param  [in]    desc_name   説明文文字列
    //! @param  [in]    parent_type 親クラスの型情報(親がないクラスはnullptr)
    Type(const char* class_name, size_t class_size, const char* desc_name, Type* parent_type = nullptr);

    //  インスタンスを作成(クラスをnewしてポインタを返す)
    virtual void* createInstance() const;

    //  クラス名を取得
    const char* className() const;

    //  説明文字列を取得
    const char* descName() const;

    //  クラスのサイズを取得
    size_t classSize() const;

    //  親ノードを取得
    //! @return ノードへのポインタ (存在しない場合はnullptr)
    Type* parent() const;

    //  子ノードを取得
    //! @return ノードへのポインタ (存在しない場合はnullptr)
    Type* child() const;

    //  次の兄弟ノードを取得
    //! @return ノードへのポインタ (存在しない場合はnullptr)
    Type* siblings() const;

private:
    //----------------------------------------------------------
    //! @name   copy/move禁止
    //----------------------------------------------------------
    //@{

    Type(const Type&)           = delete;
    Type(Type&&)                = delete;
    void operator=(const Type&) = delete;
    void operator=(Type&&)      = delete;

    //@}

private:
    const char* class_name_;       //!< クラス名
    const char* desc_name_;        //!< 説明文
    size_t      class_size_ = 0;   //!< クラスのサイズ

    //----------------------------------------------------------
    //! @name   クラス階層のツリー構造
    //! この手法で生成するツリー構造は初期化順序が不同でも構築できる
    //! 但しグローバル変数領域に定義する場合のみ利用可能。
    //!
    //! @attention 変数の初期値は意図的に設定していません。グローバル変数の0クリア仕様で動作を想定しています。
    //! @attention ここにnullptr初期化を記述すると起動時初期化順序によっては上書きされてしまうため注意
    //----------------------------------------------------------
    //@{

    Type* parent_;     //!< 親ノード
    Type* child_;      //!< 子ノード
    Type* siblings_;   //!< 次の兄弟ノード

    //@}
};

//===========================================================================
//! クラス用型情報 ClassType<T>
//===========================================================================
template <class T>
class ClassType : public Type
{
public:
    using Type::Type;   // 継承コンストラクタ

    // インスタンスを作成(クラスをnewしてポインタを返す)
    virtual void* createInstance() const override { return T::createInstance(); }

private:
};

//---------------------------------------------------------------------------
//! クラス内ヘッダー宣言 基底クラス用
//! publicで BP_BASE_TYPE(クラス名) で宣言します
//!
//! @code
//!     class A
//!     {
//!     public:
//!         BP_BASE_TYPE(A)
//! @endcode
//---------------------------------------------------------------------------
#define BP_BASE_TYPE(CLASS)                                                                                            \
    using MyClass = CLASS; /*! 自クラスの型 */                                                                   \
                                                                                                                       \
    /*! 型情報 */                                                                                                   \
    static ClassType<CLASS> TypeInfo;                                                                                  \
                                                                                                                       \
    /*! 型情報を取得 */                                                                                          \
    virtual const Type* typeInfo() const                                                                               \
    {                                                                                                                  \
        return &TypeInfo;                                                                                              \
    }                                                                                                                  \
                                                                                                                       \
    /*! インスタンスを作成(クラスをnewしてポインタを返す)*/                                      \
    static void* createInstance();

//---------------------------------------------------------------------------
//! クラス内ヘッダー宣言
//! publicで BP_BASE_TYPE(クラス名, 親クラス名) で宣言します
//!
//! @code
//!     class B : public A
//!     {
//!     public:
//!         BP_CLASS_TYPE(B, A);
//! @endcode
//---------------------------------------------------------------------------
#define BP_CLASS_TYPE(CLASS, PARENT_CLASS)                                                                             \
    using Super = PARENT_CLASS; /* 親クラスの型 */                                                               \
    BP_BASE_TYPE(CLASS);

//---------------------------------------------------------------------------
//! クラス内cpp宣言
//! グローバル変数で BP_CLASS_IMPL(クラス名, u8任意の名前文字列) で宣言します
//! @code
//! BP_CLASS_IMPL(B, u8"クラスB")
//! @endcode
//---------------------------------------------------------------------------
#define BP_CLASS_IMPL(CLASS, DESC_NAME)                                                                                \
    /*! 型情報の実体 */                                                                                          \
    ClassType<CLASS> CLASS::TypeInfo(#CLASS, sizeof(CLASS), DESC_NAME, &Super::TypeInfo);                              \
    void*            CLASS::createInstance()                                                                           \
    {                                                                                                                  \
        return new CLASS();                                                                                            \
    }

//---------------------------------------------------------------------------
//! クラス内cpp宣言 基底クラス用
//! グローバル変数で BP_BASE_IMPL(クラス名, u8任意の名前文字列) で宣言します
//! @code
//! BP_BASE_IMPL(A, u8"基底クラス")
//! @endcode
//---------------------------------------------------------------------------
#define BP_BASE_IMPL(CLASS, DESC_NAME)                                                                                 \
    /*! 型情報の実体 */                                                                                          \
    ClassType<CLASS> CLASS::TypeInfo(#CLASS, sizeof(CLASS), DESC_NAME, nullptr);                                       \
    void*            CLASS::createInstance()                                                                           \
    {                                                                                                                  \
        return new CLASS();                                                                                            \
    }

// 抽象クラス用カスタム
#define BP_BASE_IMPL_ABSOLUTE(CLASS, DESC_NAME)                                                                        \
    /*! 型情報の実体 */                                                                                          \
    ClassType<CLASS> CLASS::TypeInfo(#CLASS, sizeof(CLASS), DESC_NAME, nullptr);                                       \
    void*            CLASS::createInstance()                                                                           \
    {                                                                                                                  \
        return nullptr;                                                                                                \
    }

//===========================================================================
//! @name   ユーティリティー
//===========================================================================
//@{

//! 名前を指定して指定基底クラス型でnewする
//! @param  [in]    class_name  クラス名
//! @param  [in]    base_type   基底クラスの型情報
//! @return newされたクラスオブジェクト(失敗の場合はnullptr)
[[nodiscard]] void* CreateInstanceFromName(std::string_view class_name, Type& base_type);

//! 名前を指定して指定基底クラス型でnewする (テンプレートヘルパー)
//! @param  [in]    class_name  クラス名
//! @tparam [in]    T           基底クラスの型
//! @return newされたクラスオブジェクト(失敗の場合はnullptr)
template <class T>
[[nodiscard]] T* CreateInstanceFromName(std::string_view class_name)
{
    return reinterpret_cast<T*>(CreateInstanceFromName(class_name, T::TypeInfo));
}

//  ルートの型情報を取得する
//! @details 基底クラスはルート型情報の子になっています
Type* GetRootTypeInfo();

//@}
