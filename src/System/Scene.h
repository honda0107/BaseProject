//---------------------------------------------------------------------------
//! @file   Scene.h
//! @brief  シーン
//---------------------------------------------------------------------------
#pragma once

#include <System/Object.h>
#include <System/Component/ComponentTransform.h>
#include <System/Component/ComponentCamera.h>
#include <System/Utils/HelperLib.h>
#include <System/Cereal.h>

#include <vector>
#include <memory>
#include <sigslot/signal.hpp>

#include <iostream>
#include <sstream>
#include <string>

class Scene
{
public:
    // USING_PTR( Base );
    class Base;
    using BasePtr = std::shared_ptr<Base>;
    //  using BaseWeakPtr = std::weak_ptr < obj>;
    //	using BaseUniquePtr	= std::unique_ptr< obj >;
    using BasePtrVec = std::vector<BasePtr>;
    //	using BaseWeakPtrVec	= std::vector< BaseWeakPtr >;
    //	using BaseUniquePtrVec = std::vector< BaseUniquePtr >;
    //	using BaseVec			= std::vector< Base* >;
    using BasePtrMap = std::unordered_map<std::string, BasePtr>;

    //---------------------------------------------------------------------------
    //! シーンステータス
    //---------------------------------------------------------------------------
    enum struct StatusBit : u64
    {
        Initialized,           //!< 初期化済み
        Serialized,            //!< シリアライズ済み.
        AliveInAnotherScene,   //!< 別シーン移行でも終了しない
    };

    //---------------------------------------------------------------------------
    // シグナル
    //---------------------------------------------------------------------------
    using SignalsUpdate  = sigslot::signal<float>;
    using SignalsDefault = sigslot::signal<>;

    using SignalsDraw    = sigslot::signal<>;
    using SignalsPrePost = sigslot::signal<>;
    using SignalsGUI     = sigslot::signal<>;

    // ユーザーシグナル
    using SignalsShadow  = sigslot::signal<>;
    using SignalsGbuffer = sigslot::signal<>;
    using SignalsLight   = sigslot::signal<>;
    using SignalsHDR     = sigslot::signal<>;

    using SignalsPrePhysics = sigslot::signal<>;

    //---------------------------------------------------------------------------
    //! シーンベース
    //---------------------------------------------------------------------------
    class Base
    {
        friend class Scene;

    public:
        BP_BASE_TYPE(Scene::Base);

        Base();
        virtual ~Base();

        virtual std::string Name() = 0;

        //----------------------------------------------------------------------
        //! @name ユーザー処理
        //----------------------------------------------------------------------
        //@{

        virtual bool Init() { return true; };                  //!< 初期化
        virtual void Update([[maybe_unused]] float delta){};   //!< 更新
        virtual void Draw(){};                                 //!< 描画
        virtual void Exit(){};                                 //!< 終了
        virtual void GUI(){};                                  //!< GUI表示

        virtual void PreUpdate([[maybe_unused]] float delta){};    //!< 更新前処理
        virtual void PostUpdate([[maybe_unused]] float delta){};   //!< 更新後処理

        virtual void PreDraw([[maybe_unused]] float delta){};    //!< 描画前処理
        virtual void PostDraw([[maybe_unused]] float delta){};   //!< 描画後の処理

        virtual void InitSerialize(){};

        //@}

        //----------------------------------------------------------------------
        //! @name オブジェクト登録/削除 処理
        //----------------------------------------------------------------------
        //@{

        //! @brief オブジェクト仮登録
        //! @param obj オブジェクト
        //! @param update 更新プライオリティ
        //! @param draw 描画プライオリティ
        void PreRegister(ObjectPtr obj, Priority update = Priority::NORMAL, Priority draw = Priority::NORMAL);

        void Register(ObjectPtr obj, Priority update = Priority::NORMAL, Priority draw = Priority::NORMAL);
        void Unregister(ObjectPtr obj);
        void UnregisterAll();

        //@}
        //----------------------------------------------------------------------
        //! @name 処理優先変更 処理
        //----------------------------------------------------------------------
        //@{

        //! 優先を設定変更します
        void SetPriority(ObjectPtr obj, ProcTiming timing, Priority priority);

        //! 優先を設定変更します
        void SetPriority(ComponentPtr commponent, ProcTiming timing, Priority priority);

        //@}
        //----------------------------------------------------------------------
        //! @name シーン取得or作成/ 解放 処理
        //----------------------------------------------------------------------
        //@{

        //! シーンを作成または取得します
        //! @param T 取得するシーンクラス
        //! @return シーン
        template <class T>
        static std::shared_ptr<T> GetScene();

        //! シーンを消去します
        //! @param name シーン名
        template <class T>
        static void ReleaseScene();

        //! 確保しているものに同じシーンタイプがいないかチェックする
        static bool IsSceneExist(const BasePtr& scene);

        //! シーン確保 (GetSceneを使ってない場合の対処)
        static void SetScene(const BasePtr& scene);

        static void ReleaseScene(const BasePtr& scene);
        //@}
        //----------------------------------------------------------------------
        //! @name オブジェクト作成/取得 処理
        //----------------------------------------------------------------------
        //@{

        //! 存在するオブジェクトの取得
        //! @tparam [in] class T 取得するオブジェクトタイプ
        template <class T>
        std::shared_ptr<T> GetObjectPtr(std::string_view name = "");

        //! 存在する複数オブジェクトの取得
        //! @tparam [in] class T 取得するオブジェクトタイプ
        template <class T>
        std::vector<std::shared_ptr<T>> GetObjectsPtr();

        template <class T>
        std::shared_ptr<T> GetObjectPtrWithCreate(std::string_view name = "");

        //! 複数オブジェクト配列の取得
        const ObjectPtrVec GetObjectPtrVec() { return objects_; }

        //@}
        //----------------------------------------------------------------------
        //! @name シーン状態にかかわる 処理
        //----------------------------------------------------------------------
        //@{
        enum struct StatusBit : u64
        {
            Initialized,           //!< 初期化済み
            Serialized,            //!< シリアライズ済み.
            AliveInAnotherScene,   //!< 別シーン移行でも終了しない
        };

        void SetStatus(StatusBit b, bool on) { on ? status_.on(b) : status_.off(b); }
        bool GetStatus(StatusBit b) { return status_.is(b); }

        //! 別のシーンで生存するように設定する
        void AliveInAnotherScene(bool alive = true) { SetStatus(StatusBit::AliveInAnotherScene, alive); }

        //! 別のシーンで生存するように設定されているか?
        bool IsAliveInAnotherScene() { return GetStatus(StatusBit::AliveInAnotherScene); }

        //@}
        //----------------------------------------------------------------------
        //! @name シグナル
        //----------------------------------------------------------------------
        //@{

        //! Updateシグナル取得
        SignalsUpdate& GetUpdateSignals(ProcTiming timing)
        {
            if(timing == ProcTiming::LateUpdate)
                return signals_late_update_;

            if(timing == ProcTiming::Update)
                return signals_update_;

            assert(!"このタイミングは、GetUpdateSignals()では取得できません.");
        }

        SignalsDefault& GetSignals(ProcTiming timing)
        {
            if(timing == ProcTiming::PreUpdate)
                return signals_pre_update_;
            if(timing == ProcTiming::PostUpdate)
                return signals_post_update_;
            if(timing == ProcTiming::Update)
                assert(!"Updateは、GetUpdateSignals()で取得してください.");
            if(timing == ProcTiming::LateUpdate)
                assert(!"LateUpdateは、GetUpdateSignals()で取得してください.");

            if(timing == ProcTiming::PreDraw)
                return signals_pre_draw_;
            if(timing == ProcTiming::Draw)
                return signals_draw_;
            if(timing == ProcTiming::LateDraw)
                return signals_late_draw_;
            if(timing == ProcTiming::PostDraw)
                return signals_post_draw_;

            if(timing == ProcTiming::Light)
                return signals_light_;
            if(timing == ProcTiming::Gbuffer)
                return signals_gbuffer_;
            if(timing == ProcTiming::Shadow)
                return signals_shadow_;
            if(timing == ProcTiming::HDR)
                return signals_hdr_;

            if(timing == ProcTiming::PrePhysics)
                return signals_pre_physics_;

            assert(!"型が不明です.");
            return signals_draw_;
        }

        //@}
        //--------------------------------------------------------------------
        //! @name ユーザーシグナル
        //--------------------------------------------------------------------
        //! Shadowタイミングシグナル取得
        SignalsShadow& GetSignalsShadow() { return signals_shadow_; }
        //! Gbufferタイミングシグナル取得
        SignalsGbuffer& GetSignalsGbuffer() { return signals_gbuffer_; }
        //! Lightタイミングシグナル取得
        SignalsLight& GetSignalsLight() { return signals_light_; }
        //! Lightタイミングシグナル取得
        SignalsHDR& GetSignalsHDR() { return signals_hdr_; }

        //----------------------------------------------------------------------
        //! @name Cereal セーブロード
        //----------------------------------------------------------------------
        //@{

        virtual void Save()
        {
#if 1
            HelperLib::File::CreateFolder(".\\data\\_save\\");
            std::string   name = ".\\data\\_save\\" + Name() + ".txt";
            std::ofstream file(name);
            if(!file)
                return;

            // 存在するオブジェクトをセーブする
            {
                cereal::JSONOutputArchive o_archive(file);
                {
                    o_archive(CEREAL_NVP(objects_), CEREAL_NVP(status_.get()), CEREAL_NVP(time_));
                }
            }
            file.close();
#endif
        }

        virtual void Load()
        {
#if 1
            std::string   name = ".\\data\\_save\\" + Name() + ".txt";
            std::ifstream file(name);
            if(!file)
                return;

            // 存在するオブジェクトをセーブする
            {
                cereal::JSONInputArchive i_archive(file);
                {
                    i_archive(CEREAL_NVP(objects_), CEREAL_NVP(status_.get()), CEREAL_NVP(time_));
                }
                // 処理のシリアライズは再度行う
                status_.off(StatusBit::Serialized);
            }
#endif
        }
        //@}

    private:
#if 0
		//@{
		CEREAL_SAVELOAD( arc, ver )
		{
			arc( CEREAL_NVP( objects_ ), CEREAL_NVP( status_.get() ), CEREAL_NVP( time_ ) );
		}
		//@}
#endif

        //! オブジェクトの指定処理を指定優先で処理するように登録する
        template <class T>
        void setProc(ObjectPtr obj, SlotProc<T> slot);

        //! オブジェクトの指定処理を削除する
        template <class T>
        void resetProc(ObjectPtr obj, SlotProc<T> slot);

        //! @brief コンポーネントの指定処理を指定優先で処理するように登録する
        template <class T>
        void setProc(ComponentPtr component, SlotProc<T> slot);

        //! @brief コンポーネントの指定処理を削除する
        template <class T>
        void resetProc(ComponentPtr component, SlotProc<T> slot);

        ObjectPtrVec      pre_objects_;   //!< シーンに存在させるオブジェクト(仮登録)
        ObjectPtrVec      objects_;       //!< シーンに存在するオブジェクト
        Status<StatusBit> status_;        //!< 状態

        SignalsPrePost signals_pre_update_;
        SignalsUpdate  signals_update_;
        SignalsUpdate  signals_late_update_;
        SignalsPrePost signals_post_update_;

        SignalsPrePost signals_pre_draw_;
        SignalsDraw    signals_draw_;
        SignalsDraw    signals_late_draw_;
        SignalsPrePost signals_post_draw_;

        // ユーザーシグナル
        SignalsShadow  signals_shadow_;
        SignalsGbuffer signals_gbuffer_;
        SignalsLight   signals_light_;
        SignalsHDR     signals_hdr_;

        SignalsPrePhysics signals_pre_physics_;

        bool  pause_    = false;   //!< ポーズ中
        bool  step_     = false;   //!< 1フレームスキップ
        float time_     = 0.0f;    //!< タイマー
        float over_lap_ = 0.0f;    //!< シーン切り替えオーバーラップ

        bool change_next_ = false;   //!< 次のシーンへ移行する
    };

    //----------------------------------------------------------------
    //! @name シーン操作関係
    //----------------------------------------------------------------
    //@{

    //! シーンを作成または取得します
    //! @param [in] class T 取得するシーンクラス
    //! @param [in] name シーン名
    //! @return シーン
    template <class T>
    static std::shared_ptr<T> GetScene()
    {
        // 存在する場合はシーンを返す
        // あまりよろしくはないがシーンを作成し名前を取得する
        T    forName;
        auto name = forName.Name();
        if(scenes_.count(name) > 0) {
            return std::dynamic_pointer_cast<T>(scenes_[name]);
        }

        // 存在しない場合は作成します
        auto scene    = std::make_shared<T>();
        scenes_[name] = scene;
        return scene;
    }

    //! シーンを消去します
    //! @param [in] name シーン名
    template <class T>
    static void ReleaseScene()
    {
        Base::ReleaseScene<T>();
    }

    static void ReleaseScene(const ::Scene::BasePtr& scene)
    {
        ::Scene::Base::ReleaseScene(scene);
    }

    //! シーン切り替え
    //! @param scene 次に再生するシーン
    //! @details 内部的には、SetNextScene() -> ChangeNextScene() が呼ばれています
    static void Change(BasePtr scene);

    //! 次のシーンをセットする
    static void SetNextScene(BasePtr scene);

    //! 次のシーンに切り替える
    static void ChangeNextScene();

    //  現在アクティブなシーンを取得します
    static Scene::Base* GetCurrentScene();

    //@}
    //----------------------------------------------------------------
    //! @name オブジェクト操作 関係
    //----------------------------------------------------------------
    //@{

    //! オブジェクトの作成
    //! Objectをシーン上に発生させる
    //! @param no_transform ComponentTransformを作らない (true = 作らない)
    //! @param update 処理優先
    //! @param draw 描画優先
    template <class T>
    static std::shared_ptr<T>
    CreateObject(bool no_transform = false, Priority update = Priority::NORMAL, Priority draw = Priority::NORMAL)
    {
        if(current_scene_) {
            auto tmp = std::make_shared<T>();
            current_scene_->PreRegister(tmp, update, draw);

            // デフォルトではComponentTransformは最初から用意する
            if(!no_transform)
                tmp->AddComponent<ComponentTransform>();

            // Object::Init()は作成したときに行うように変更
            bool ret = tmp->Init();
            if(ret) {
                assert("継承先のInit()にて__super::Init()を入れてください." &&
                       tmp->GetStatus(Object::StatusBit::Initialized));
            }
            return tmp;
        }
        assert(!"Scene上で作成しなければなりません.");
        return std::shared_ptr<T>();
    }

    template <class T>
    static void ReleaseObject()
    {
        auto obj = current_scene_->GetObjectPtr<T>();
        current_scene_->Unregister(obj);
    }

    static void ReleaseObject(std::string_view name = "");

    static void ReleaseObject(ObjectPtr obj);

    //@}
    //----------------------------------------------------------------
    //! @name シーン処理 関係
    //----------------------------------------------------------------
    //@{

    //! 初期化処理
    static void Init();

    //! @brief シーン更新前処理
    //! @detail 基本的にはUpdate集団より先にで行いたいものをこの層で処理します
    static void PreUpdate();

    //! シーン更新
    //! @param [in] delta 更新インターバル(秒指定)
    static void Update(float delta);

    //! @brief Physics前処理
    //! @detail OwnerObject移動によるコリジョンコンポーネント付随処理
    static void PrePhysics();

    //! @brief シーン更新後の処理
    //! @detail Physicsおよびすべての当たり判定後に処理します
    static void PostUpdate();

    //! シーン描画
    static void Draw();

    //! シーン終了
    static void Exit();

    //! @brief シーンGUI
    //! @detail ※この処理はUpdate後に行う必要があります
    static void GUI();

    //@}
    //----------------------------------------------------------------
    //! @name シーン状態 関係
    //----------------------------------------------------------------
    //@{

    //! ポーズ中かをチェック
    //! @retval true ポーズ中
    //! @retval false ポーズしていない
    static bool IsPause();

    //! シーン内時間の取得
    //! @return シーンが始まってからの時間
    static float GetTime();

    //! 存在するシーン数
    static size_t GetSceneCount()
    {
        return scenes_.size();
    }

    //! @brief エディター状態設定
    //! @param edit Editor状態
    static void SetEdit(bool edit);

    //! @brief エディター状態取得
    //! @retval true: エディター状態
    static const bool IsEdit();

    //@}
    //----------------------------------------------------------------
    //! @name シーン内オブジェクト取得 関係
    //----------------------------------------------------------------
    //@{

    //! @brief オブジェクトサーチ&取得
    //! @tparam T 取得したいオブジェクトタイプ
    //! @param name 取得したいオブジェクトの名前
    //! @return オブジェクト
    template <class T>
    static std::shared_ptr<T> GetObjectPtr(std::string_view name = "")
    {
        //! @todo nullptrの時、存在しないので、ここでLOGでエラーを出しておく
        return current_scene_->GetObjectPtr<T>(name);
    }

    //! @brief オブジェクトサーチ&取得
    //! @tparam T 取得したいオブジェクトタイプ
    //! @details 同じタイプのオブジェクトが複数あると先に見つかったものを返します
    //! @return オブジェクト
    template <class T>
    static std::vector<std::shared_ptr<T>> GetObjectsPtr()
    {
        return current_scene_->GetObjectsPtr<T>();
    }

    template <class T>
    static std::shared_ptr<T> GetObjectPtrWithCreate(std::string_view name = "")
    {
        return current_scene_->GetObjectPtrWithCreate<T>(name);
    }

    //! @brief ComponentCollisionの当たり判定を行う
    static void CheckComponentCollisions();

    //! セレクトしているオブジェクトかをチェックする
    static bool SelectObjectWindow(const ObjectPtr& object);

    //! @brief
    //! GUIオブジェクトのEditor設定であるためObjectでなくこちらで設定する
    static void SetGUIObjectDetailSize();
    //@}
    //----------------------------------------------------------------------
    //! @name Proc
    //----------------------------------------------------------------------
    //@{

public:
    static SignalsDefault& GetSignals(ProcTiming timing)
    {
        return current_scene_->GetSignals(timing);
    }

    static SignalsUpdate& GetUpdateSignals(ProcTiming timing)
    {
        return current_scene_->GetUpdateSignals(timing);
    }

    //@}

    static const int SETGUI_ATTACH_LEFT   = -10000;
    static const int SETGUI_ATTACH_RIGHT  = +10000;
    static const int SETGUI_ATTACH_TOP    = -10000;
    static const int SETGUI_ATTACH_BOTTOM = +10000;
    static void      SetGUIWindow(int posx, int posy, int width, int height);

    //! @brief オブジェクト選択
    //! @param x mouseポインタX
    //! @param y mouseポインタY
    //! @return ObjectPtr (ない場合はnullptr)
    static ObjectPtr PickObject(int x, int y);

    //! @brief カレントカメラの取得
    //! @return カレントカメラ
    static ComponentCameraWeakPtr GetCurrentCamera();

    //! @brief オブジェクトの名前でのカレントカメラの設定
    //! @brief name 名前
    //! @return カレントカメラ
    static ComponentCameraWeakPtr SetCurrentCamera(const std::string_view name);

    enum struct EditorStatusBit : u64
    {
        EditorPlacement,          //!< エディタ配置
        EditorPlacement_Always,   //!< エディタ配置(常に引っ付く)
    };
    static void SetEditorStatus(EditorStatusBit b, bool on)
    {
        editor_status_.set(b, on);
    }
    static bool GetEditorStatus(EditorStatusBit b)
    {
        return editor_status_.is(b);
    }
    static Status<EditorStatusBit>& SceneStatus()
    {
        return editor_status_;
    }

    //----------------------------------------------------------------------
    //! @name Cereal セーブロード
    //----------------------------------------------------------------------
    //@{

    static void SaveEditor()
    {
        HelperLib::File::CreateFolder(".\\data\\_save\\");
        std::string   name = ".\\data\\_save\\Editor.txt";
        std::ofstream file(name);
        if(!file)
            return;

        // 存在するオブジェクトをセーブする
        {
            cereal::JSONOutputArchive o_archive(file);
            {
                o_archive(CEREAL_NVP(editor_status_.get()));
                o_archive(CEREAL_NVP(inspector_size));
                o_archive(CEREAL_NVP(object_detail_size));
            }
        }
        file.close();
    }

    static void LoadEditor()
    {
        std::string   name = ".\\data\\_save\\Editor.txt";
        std::ifstream file(name);
        if(!file)
            return;

        // 存在するオブジェクトをセーブする
        std::stringstream ss;
        {
            cereal::JSONInputArchive i_archive(file);
            {
                i_archive(CEREAL_NVP(editor_status_.get()));
                i_archive(CEREAL_NVP(inspector_size));
                i_archive(CEREAL_NVP(object_detail_size));
            }
        }
    }
    //@}

private:
    static Status<EditorStatusBit> editor_status_;   //!< 状態
    static float2                  inspector_size;
    static float2                  object_detail_size;

private:
    //! @brief 関数シリアライズ (InitSerializeの呼び出し)
    static void functionSerialize(ObjectPtr obj);

    // シリアライズされてないものがないかチェックします
    static void checkSerialized(ObjectPtr obj);
    static void checkSerialized(ComponentPtr comp);

    static void checkNextAlive();

    static BasePtr current_scene_;   //!< 現在のシーン
    static BasePtr next_scene_;      //!< 変更シーン

    static BasePtrMap scenes_;   //!< 存在する全シーン
};

//! @brief オブジェクト取得
//! @tparam T 取得したいオブジェクトクラス
//! @return 取得オブジェクト
template <class T>
std::shared_ptr<T> Scene::Base::GetObjectPtr(std::string_view name)
{
    if(name.empty()) {
        for(auto& obj : objects_) {
            auto cast = std::dynamic_pointer_cast<T>(obj);

            if(cast)
                return cast;
        }
    }
    else {
        for(auto& obj : objects_) {
            if(name.compare(obj->GetNameDefault()) == 0) {
                auto cast = std::dynamic_pointer_cast<T>(obj);

                if(cast)
                    return cast;
            }
        }
        for(auto& obj : objects_) {
            if(name.compare(obj->GetName()) == 0) {
                auto cast = std::dynamic_pointer_cast<T>(obj);

                if(cast)
                    return cast;
            }
        }

        // 作成前Objectも検査する
        for(auto& obj : pre_objects_) {
            if(name.compare(obj->GetNameDefault()) == 0) {
                auto cast = std::dynamic_pointer_cast<T>(obj);

                if(cast)
                    return cast;
            }
        }
        for(auto& obj : pre_objects_) {
            if(name.compare(obj->GetName()) == 0) {
                auto cast = std::dynamic_pointer_cast<T>(obj);

                if(cast)
                    return cast;
            }
        }
    }

    return nullptr;
}

//! 複数オブジェクトの取得
//! @param <class T> 取得したいオブジェクトクラス
//! @return 複数の指定オブジェクト
template <class T>
std::vector<std::shared_ptr<T>> Scene::Base::GetObjectsPtr()
{
    std::vector<std::shared_ptr<T>> objects;

    for(auto& obj : objects_) {
        auto cast = std::dynamic_pointer_cast<T>(obj);
        if(cast)
            objects.push_back(cast);
    }
    return objects;
}

template <class T>
std::shared_ptr<T> Scene::Base::GetObjectPtrWithCreate(std::string_view name)
{
    std::shared_ptr<T> ptr = std::dynamic_pointer_cast<T>(Scene::GetCurrentScene()->GetObjectPtr<T>(name));
    if(ptr == nullptr) {
        ptr = std::dynamic_pointer_cast<T>(Scene::CreateObject<T>()->SetName(std::string(name)));
    }
    return ptr;
}
