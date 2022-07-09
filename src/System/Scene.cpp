#include "Scene.h"
#include "ProcTiming.h"

#include <System/Object.h>
#include <System/Component/ComponentModel.h>
#include <System/Component/ComponentCollision.h>
#include <System/Debug/DebugCamera.h>
#include <System/SystemMain.h>   // ResetDeltaTime

#include <algorithm>

//=============================================================
// シーン ローカル変数
//=============================================================
BP_BASE_IMPL_ABSOLUTE(Scene::Base, u8"シーン基底");

namespace
{
bool  scene_edit    = true;    //!< エディター状態
bool  scene_pause   = false;   //!< ポーズ中
bool  scene_step    = false;   //!< 1フレームスキップ
float scene_time    = 0.0f;    //!< タイマー
float scene_overlap = 0.0f;    //!< シーン切り替えオーバーラップ

bool scene_change_next = false;   //!< 次のシーンへ移行する

int                   select_object_index = 0;   //!< GUIでセレクトされているオブジェクト
std::weak_ptr<Object> selectObject;

#if 1
auto BindObjectUpdate(ProcTiming proc, ObjectPtr obj)
{
    if(ProcTiming::Update == proc)
        return std::bind(&Object::Update, obj.get(), std::placeholders::_1);
    if(ProcTiming::LateUpdate == proc)
        return std::bind(&Object::LateUpdate, obj.get(), std::placeholders::_1);

    assert(!"処理がBindできません");
    return std::bind(&Object::Update, obj.get(), std::placeholders::_1);
}

auto BindObjectProc(ProcTiming proc, ObjectPtr obj)
{
    if(ProcTiming::PreUpdate == proc)
        return std::bind(&Object::PreUpdate, obj.get());
    if(ProcTiming::PostUpdate == proc)
        return std::bind(&Object::PostUpdate, obj.get());
    if(ProcTiming::PreDraw == proc)
        return std::bind(&Object::PreDraw, obj.get());
    if(ProcTiming::Draw == proc)
        return std::bind(&Object::Draw, obj.get());
    if(ProcTiming::LateDraw == proc)
        return std::bind(&Object::LateDraw, obj.get());
    if(ProcTiming::PostDraw == proc)
        return std::bind(&Object::PostDraw, obj.get());

    if(ProcTiming::PrePhysics == proc)
        return std::bind(&Object::PrePhysics, obj.get());

#if 0
		if ( ProcTiming::Shadow == proc )
			return std::bind( &Object::Shadow, obj.get() );
		if ( ProcTiming::Gbuffer == proc )
			return std::bind( &Object::Gbuffer, obj.get() );
		if ( ProcTiming::Light == proc )
			return std::bind( &Object::Light, obj.get() );
		if ( ProcTiming::HDR == proc )
			return std::bind( &Object::HDR, obj.get() );
#endif
    // ない場合は一旦Drawで
    assert(!"処理がBindできません");
    return std::bind(&Object::Draw, obj.get());
}
#endif

#if 1
auto BindComponentUpdate(ProcTiming proc, ComponentPtr cmp)
{
    if(ProcTiming::Update == proc)
        return std::bind(&Component::Update, cmp.get(), std::placeholders::_1);
    if(ProcTiming::LateUpdate == proc)
        return std::bind(&Component::LateUpdate, cmp.get(), std::placeholders::_1);

    assert(!"処理がBindできません");
    return std::bind(&Component::Update, cmp.get(), std::placeholders::_1);
}

auto BindComponentProc(ProcTiming proc, ComponentPtr cmp)
{
    if(ProcTiming::PreUpdate == proc)
        return std::bind(&Component::PreUpdate, cmp.get());
    if(ProcTiming::PostUpdate == proc)
        return std::bind(&Component::PostUpdate, cmp.get());
    if(ProcTiming::PreDraw == proc)
        return std::bind(&Component::PreDraw, cmp.get());
    if(ProcTiming::Draw == proc)
        return std::bind(&Component::Draw, cmp.get());
    if(ProcTiming::LateDraw == proc)
        return std::bind(&Component::LateDraw, cmp.get());
    if(ProcTiming::PostDraw == proc)
        return std::bind(&Component::PostDraw, cmp.get());

    if(ProcTiming::PrePhysics == proc)
        return std::bind(&Component::PrePhysics, cmp.get());

#if 0
		if ( ProcTiming::Shadow == proc )
			return std::bind( &Object::Shadow, cmp.get() );
		if ( ProcTiming::Gbuffer == proc )
			return std::bind( &Object::Gbuffer, cmp.get() );
		if ( ProcTiming::Light == proc )
			return std::bind( &Object::Light, cmp.get() );
		if ( ProcTiming::HDR == proc )
			return std::bind( &Object::HDR, cmp.get() );
#endif
    // ない場合は一旦Drawで
    assert(!"処理がBindできません");
    return std::bind(&Component::Draw, cmp.get());
}
#endif

ObjectWeakPtrVec leak_objs;

}   // namespace

Scene::BasePtr                 Scene::current_scene_ = nullptr;   //!< 現在のシーン
Scene::BasePtr                 Scene::next_scene_    = nullptr;   //!< 変更シーン
Scene::BasePtrMap              Scene::scenes_        = {};        //!< 存在する全シーン
Status<Scene::EditorStatusBit> Scene::editor_status_;             //!< シーン状態
float2                         Scene::inspector_size{300, 300};
float2                         Scene::object_detail_size{300, 452};

//=============================================================
// シーンクラス処理
//=============================================================

//! 初期化処理
Scene::Base::Base()
{
}

//! 終了処理
Scene::Base::~Base()
{
    UnregisterAll();
}

//! @brief 優先を設定変更します
//! @param obj 優先を変更したいオブジェクト
//! @param timing 固定処理
//! @param priority 設定する優先
void Scene::Base::SetPriority(ObjectPtr obj, ProcTiming timing, Priority priority)
{
    // 以前いるプライオリティから削除し、
    // 設定したい優先に設定する
    if(timing == ProcTiming::Update || timing == ProcTiming::LateUpdate) {
        auto& proc     = obj->GetProc<float>(GetProcTimingName(timing), timing);
        proc.timing_   = timing;
        proc.priority_ = priority;
        proc.proc_     = BindObjectUpdate(timing, obj);
        resetProc(obj, proc);
        setProc(obj, proc);
    }
    else {
        auto& proc     = obj->GetProc<void>(GetProcTimingName(timing), timing);
        proc.timing_   = timing;
        proc.priority_ = priority;
        proc.proc_     = BindObjectProc(timing, obj);
        resetProc(obj, proc);
        setProc(obj, proc);
    }
}

//!	@brief 優先を設定変更します
//!	@param component 優先を変更したいコンポーネント
//!	@param timing 固定処理
//!	@param priority 設定する優先
void Scene::Base::SetPriority(ComponentPtr component, ProcTiming timing, Priority priority)
{
    // 以前いるプライオリティから削除し、
    // 設定したい優先に設定する
    if(timing == ProcTiming::Update || timing == ProcTiming::LateUpdate) {
        auto& proc     = component->GetProc<float>(GetProcTimingName(timing), timing);
        proc.timing_   = timing;
        proc.priority_ = priority;
        proc.proc_     = BindComponentUpdate(timing, component);
        resetProc(component, proc);
        setProc(component, proc);
    }
    else {
        auto& proc     = component->GetProc<void>(GetProcTimingName(timing), timing);
        proc.timing_   = timing;
        proc.priority_ = priority;
        proc.proc_     = BindComponentProc(timing, component);
        resetProc(component, proc);
        setProc(component, proc);
    }
}

void Scene::Base::PreRegister(ObjectPtr obj, Priority update, Priority draw)
{
    {
        auto itr = std::find(pre_objects_.begin(), pre_objects_.end(), obj);
        // すでに存在している?
        if(itr != pre_objects_.end())
            return;
    }
    {
        auto itr = std::find(objects_.begin(), objects_.end(), obj);
        // すでに存在している?
        if(itr != objects_.end())
            return;
    }

    // 処理を追加
    auto& proc_update     = obj->GetProc<float>(GetProcTimingName(ProcTiming::Update), ProcTiming::Update);
    proc_update.priority_ = update;
    auto& proc_draw       = obj->GetProc<void>(GetProcTimingName(ProcTiming::Draw), ProcTiming::Draw);
    proc_draw.priority_   = draw;

    pre_objects_.push_back(obj);
}

void Scene::Base::Register(ObjectPtr obj, Priority update, Priority draw)
{
    auto itr = std::find(objects_.begin(), objects_.end(), obj);
    if(itr != objects_.end()) {
        // すでに存在している
        return;
    }

    // 処理を追加
    objects_.push_back(obj);

    auto& proc_preupdate     = obj->GetProc<void>(GetProcTimingName(ProcTiming::PreUpdate), ProcTiming::PreUpdate);
    proc_preupdate.timing_   = ProcTiming::PreUpdate;
    proc_preupdate.priority_ = update;
    proc_preupdate.proc_     = BindObjectProc(ProcTiming::PreUpdate, obj);
    proc_preupdate.dirty_    = false;
    setProc(obj, proc_preupdate);

    auto& proc_update     = obj->GetProc<float>(GetProcTimingName(ProcTiming::Update), ProcTiming::Update);
    proc_update.timing_   = ProcTiming::Update;
    proc_update.priority_ = update;
    proc_update.proc_     = BindObjectUpdate(ProcTiming::Update, obj);
    proc_update.dirty_    = false;
    setProc(obj, proc_update);

    auto& proc_late_update     = obj->GetProc<float>(GetProcTimingName(ProcTiming::LateUpdate), ProcTiming::LateUpdate);
    proc_late_update.timing_   = ProcTiming::LateUpdate;
    proc_late_update.priority_ = update;
    proc_late_update.proc_     = BindObjectUpdate(ProcTiming::LateUpdate, obj);
    proc_late_update.dirty_    = false;
    setProc(obj, proc_late_update);

    auto& proc_pre_physics     = obj->GetProc<void>(GetProcTimingName(ProcTiming::PrePhysics), ProcTiming::PrePhysics);
    proc_pre_physics.timing_   = ProcTiming::PrePhysics;
    proc_pre_physics.priority_ = update;
    proc_pre_physics.proc_     = BindObjectProc(ProcTiming::PrePhysics, obj);
    proc_pre_physics.dirty_    = false;
    setProc(obj, proc_pre_physics);

    auto& proc_postupdate     = obj->GetProc<void>(GetProcTimingName(ProcTiming::PostUpdate), ProcTiming::PostUpdate);
    proc_postupdate.timing_   = ProcTiming::PostUpdate;
    proc_postupdate.priority_ = update;
    proc_postupdate.proc_     = BindObjectProc(ProcTiming::PostUpdate, obj);
    proc_postupdate.dirty_    = false;
    setProc(obj, proc_postupdate);

    auto& proc_predraw     = obj->GetProc<void>(GetProcTimingName(ProcTiming::PreDraw), ProcTiming::PreDraw);
    proc_predraw.timing_   = ProcTiming::PreDraw;
    proc_predraw.priority_ = draw;
    proc_predraw.proc_     = BindObjectProc(ProcTiming::PreDraw, obj);
    proc_predraw.dirty_    = false;
    setProc(obj, proc_predraw);

    auto& proc_draw     = obj->GetProc<void>(GetProcTimingName(ProcTiming::Draw), ProcTiming::Draw);
    proc_draw.timing_   = ProcTiming::Draw;
    proc_draw.priority_ = draw;
    proc_draw.proc_     = BindObjectProc(ProcTiming::Draw, obj);
    proc_draw.dirty_    = false;
    setProc(obj, proc_draw);

    auto& proc_latedraw     = obj->GetProc<void>(GetProcTimingName(ProcTiming::LateDraw), ProcTiming::LateDraw);
    proc_latedraw.timing_   = ProcTiming::LateDraw;
    proc_latedraw.priority_ = draw;
    proc_latedraw.proc_     = BindObjectProc(ProcTiming::LateDraw, obj);
    proc_latedraw.dirty_    = false;
    setProc(obj, proc_latedraw);

    auto& proc_postdraw     = obj->GetProc<void>(GetProcTimingName(ProcTiming::PostDraw), ProcTiming::PostDraw);
    proc_postdraw.timing_   = ProcTiming::PostDraw;
    proc_postdraw.priority_ = draw;
    proc_postdraw.proc_     = BindObjectProc(ProcTiming::PostDraw, obj);
    proc_postdraw.dirty_    = false;
    setProc(obj, proc_postdraw);
}

void Scene::Base::Unregister(ObjectPtr obj)
{
    auto itr = std::find(objects_.begin(), objects_.end(), obj);

    obj->RemoveAllProcesses();

    // リストから削除( 自動deleteされる )
    (*itr)->RemoveAllComponents();
    (*itr)->ModifyComponents();
    objects_.erase(itr);
}

void Scene::Base::UnregisterAll()
{
    // オブジェクトを消去 (自動delete)
    for(auto& obj : objects_) {
        obj->RemoveAllComponents();
        obj->ModifyComponents();

        obj->RemoveAllProcesses();
    }
    for(auto obj : objects_)
        leak_objs.push_back(obj);

    objects_.clear();

    signals_update_.disconnect_all();
    signals_draw_.disconnect_all();

    signals_pre_update_.disconnect_all();
    signals_post_update_.disconnect_all();
    signals_pre_draw_.disconnect_all();
    signals_post_draw_.disconnect_all();

    // ユーザーシグナル
    signals_shadow_.disconnect_all();
    signals_gbuffer_.disconnect_all();
    signals_light_.disconnect_all();
    signals_hdr_.disconnect_all();
}

//! 同じシーンタイプがいないかチェックする
bool Scene::Base::IsSceneExist(const BasePtr& scene)
{
    auto it = scenes_.find(scene->Name());

    if(it != scenes_.end()) {
        // まだ生きている同じタイプのシーンがあり、それが自分ではない
        if(it->second != scene)
            return true;
    }

    return false;
}

void Scene::Base::SetScene(const BasePtr& scene)
{
    auto name = scene->Name();

    auto it = scenes_.find(name);
    if(it != scenes_.end()) {
        scenes_[name] = scene;
    }
}

void Scene::Base::ReleaseScene(const BasePtr& scene)
{
    auto name = scene->Name();

    auto it = scenes_.find(name);
    if(it != scenes_.end()) {
        scenes_.erase(name);
    }
}

//! @brief オブジェクトの指定処理を指定優先で処理するように登録する
//! @param obj オブジェクト
//! @param timing 指定処理
//! @param priority 指定優先
template <class T>
void Scene::Base::setProc(ObjectPtr obj, SlotProc<T> slot)
{
    // 引数の型が異なる状態になっている
    // ※ Updateは、float一つ伴います
    [[maybe_unused]] constexpr bool is_float = std::is_same<T, float>{};

    assert(((slot.GetTiming() != ProcTiming::Update && slot.GetTiming() != ProcTiming::LateUpdate) && !is_float) ||
           ((slot.GetTiming() == ProcTiming::Update || slot.GetTiming() == ProcTiming::LateUpdate) && is_float));

    // 設定したい優先に設定する
    if constexpr(std::is_same<T, float>{}) {
        auto& proc = obj->GetProc<T>(slot.GetName(), slot.GetTiming());
        proc.SetProc(slot.GetName(), slot.GetTiming(), slot.GetPriority(), slot.GetProc());
        proc.connect_ = current_scene_->GetUpdateSignals(slot.GetTiming()).connect(proc.proc_);
    }
    else {
        auto& proc = obj->GetProc<T>(slot.GetName(), slot.GetTiming());
        proc.SetProc(slot.GetName(), slot.GetTiming(), slot.GetPriority(), slot.GetProc());
        proc.connect_ = current_scene_->GetSignals(slot.GetTiming()).connect(proc.proc_);
    }
}

//! @brief オブジェクトの指定処理を削除する
//! @param obj 削除するオブジェクト
//! @param timing 指定処理
template <class T>
void Scene::Base::resetProc(ObjectPtr obj, SlotProc<T> slot)
{
    // 以前いるconnectから削除
    if(slot.connect_.valid())
        slot.connect_.disconnect();
#if 0
	auto& proc = obj->GetProc<T>( GetProcTimingName( slot.GetTiming() ), slot.GetTiming() );
	proc.connect_.disconnect();
#endif
}

//! @brief コンポーネントの指定処理を指定優先で処理するように登録する
//! @param component コンポーネント
//! @param timing 指定処理
//! @param priority 指定優先
template <class T>
void Scene::Base::setProc(ComponentPtr component, SlotProc<T> slot)
{
    // 引数の型が異なる状態になっている
    [[maybe_unused]] constexpr bool is_float = std::is_same<T, float>{};

    assert(((slot.GetTiming() != ProcTiming::Update && slot.GetTiming() != ProcTiming::LateUpdate) && !is_float) ||
           ((slot.GetTiming() == ProcTiming::Update || slot.GetTiming() == ProcTiming::LateUpdate) && is_float));

    auto& proc = component->GetProc<T>(slot.GetName(), slot.GetTiming());
    if(slot.GetProc() == nullptr) {
        if constexpr(std::is_same<T, float>{})
            slot.SetProc(slot.GetName(),
                         slot.GetTiming(),
                         slot.GetPriority(),
                         BindComponentUpdate(slot.GetTiming(), component));
        else
            slot.SetProc(slot.GetName(),
                         slot.GetTiming(),
                         slot.GetPriority(),
                         BindComponentProc(slot.GetTiming(), component));
    }

    proc.SetProc(slot.GetName(), slot.GetTiming(), slot.GetPriority(), slot.GetProc());

    // 設定したい優先に設定する
    if constexpr(std::is_same<T, float>{}) {
        proc.connect_ = current_scene_->GetUpdateSignals(slot.GetTiming()).connect(proc.proc_, (int)proc.priority_);
    }
    else {
        proc.connect_ = current_scene_->GetSignals(slot.GetTiming()).connect(proc.proc_, (int)proc.priority_);
    }
}

//! @brief コンポーネントの指定処理を削除する
//! @param component 削除するオブジェクト
//! @param timing 指定処理
template <class T>
void Scene::Base::resetProc(ComponentPtr component, SlotProc<T> slot)
{
    // 以前いるプライオリティから削除
    if(slot.connect_.valid())
        slot.connect_.disconnect();
#if 0
	auto& proc = component->GetProc<T>( GetProcTimingName( slot.GetTiming() ), slot.GetTiming() );
	proc.connect_.disconnect();
#endif
}

void Scene::functionSerialize(ObjectPtr obj)
{
    // オブジェクト
    if(!obj->GetStatus(Object::StatusBit::Serialized)) {
        obj->InitSerialize();
        assert("継承先のInitSerialize()にて__super::InitSerialize()を入れてください." &&
               obj->GetStatus(Object::StatusBit::Serialized));

        // シリアライズされない処理を確認
        checkSerialized(obj);
    }

    // コンポーネントシグナル設定
    for(auto component : obj->GetComponents()) {
        if(!component->GetStatus(Component::StatusBit::Initialized)) {
            for(ProcTiming i = ProcTiming::PreUpdate; (int)i <= (int)ProcTiming::PostDraw;
                i            = (ProcTiming)((int)i + 1)) {
                if(i == ProcTiming::Update || i == ProcTiming::LateUpdate) {
                    auto& proc = component->GetProc<float>(GetProcTimingName(i), i);
                    current_scene_->setProc(component, proc);
                    proc.ResetDirty();
                }
                else {
                    auto& proc = component->GetProc<void>(GetProcTimingName(i), i);
                    current_scene_->setProc(component, proc);
                    proc.ResetDirty();
                }
            }
            component->Init();
            assert("継承先のInit()にて__super::Init()を入れてください." &&
                   component->GetStatus(Component::StatusBit::Initialized));
        }
        if(!component->GetStatus(Component::StatusBit::Serialized)) {
            component->InitSerialize();
            assert("継承先のInitSerialize()にて__super::InitSerialize()を入れてください." &&
                   component->GetStatus(Component::StatusBit::Serialized));

            checkSerialized(component);
        }
    }
}

void Scene::checkSerialized(ObjectPtr obj)
{
    // 標準のUpdateシグナルを復旧
    for(auto& sig : obj->update_timings_) {
        auto& dat = sig.second;
        if(dat.GetName() == GetProcTimingName(dat.GetTiming()))
            dat.SetProc(dat.GetName(), dat.GetTiming(), dat.GetPriority(), BindObjectUpdate(dat.GetTiming(), obj));
    }
    // 標準のUpdate以外のシグナルを復旧
    for(auto& sig : obj->proc_timings_) {
        auto& dat = sig.second;
        if(dat.GetName() == GetProcTimingName(dat.GetTiming())) {
            dat.SetProc(dat.GetName(), dat.GetTiming(), dat.GetPriority(), BindObjectProc(dat.GetTiming(), obj));
        }
    }

    // 処理がnullptrのものがないかチェック
    for(auto& proc : obj->update_timings_) {
        [[maybe_unused]] auto& p = proc.second;
        // エラーの際は、pの名前で追加した処理を InitSerialize()に移動してください
        assert(p.GetProc() != nullptr &&
               "SetProcで追加した処理がSerializeされていません。InitSerialize()で初期化してください.");
    }

    for(auto& proc : obj->proc_timings_) {
        [[maybe_unused]] auto& p = proc.second;
        // エラーの際は、pの名前で追加した処理を InitSerialize()に移動してください
        assert(p.GetProc() != nullptr &&
               "SetProcで追加した処理がSerializeされていません。InitSerialize()で初期化してください.");
    }
}

void Scene::checkSerialized(ComponentPtr comp)
{
    // 標準のUpdateシグナルを復旧
    for(auto& sig : comp->update_timings_) {
        auto& dat = sig.second;
        if(dat.GetName() == GetProcTimingName(dat.GetTiming()))
            dat.SetProc(dat.GetName(), dat.GetTiming(), dat.GetPriority(), BindComponentUpdate(dat.GetTiming(), comp));
    }
    // 標準のUpdate以外のシグナルを復旧
    for(auto& sig : comp->proc_timings_) {
        auto& dat = sig.second;
        if(dat.GetName() == GetProcTimingName(dat.GetTiming())) {
            dat.SetProc(dat.GetName(), dat.GetTiming(), dat.GetPriority(), BindComponentProc(dat.GetTiming(), comp));
        }
    }

    // 処理がnullptrのものがないかチェック
    for(auto& proc : comp->update_timings_) {
        [[maybe_unused]] auto& p = proc.second;
        // エラーの際は、pの名前で追加した処理を InitSerialize()に移動してください
        assert(p.GetProc() != nullptr &&
               "SetProcで追加した処理がSerializeされていません。InitSerialize()で初期化してください.");
    }

    for(auto& proc : comp->proc_timings_) {
        [[maybe_unused]] auto& p = proc.second;
        // エラーの際は、pの名前で追加した処理を InitSerialize()に移動してください
        assert(p.GetProc() != nullptr &&
               "SetProcで追加した処理がSerializeされていません。InitSerialize()で初期化してください.");
    }
}

void Scene::checkNextAlive()
{
    if(current_scene_) {
        // シーン切り替えで前の状態を確保する?
        if(current_scene_->GetStatus(Base::StatusBit::AliveInAnotherScene)) {
            Scene::Base::SetScene(current_scene_);
        }
        else {
            Scene::Base::ReleaseScene(current_scene_);
        }
    }
}

//=============================================================
// シーン処理
//=============================================================
void Scene::ReleaseObject(std::string_view name)
{
    if(current_scene_) {
        auto obj = GetObjectPtr<Object>(name);
        if(obj)
            obj->SetStatus(Object::StatusBit::Alive, false);
    }
}

void Scene::ReleaseObject(ObjectPtr obj)
{
    if(current_scene_ && obj)
        obj->SetStatus(Object::StatusBit::Alive, false);
}

//! 次のシーンをセットする
void Scene::SetNextScene(BasePtr scene)
{
    checkNextAlive();

    if(scene && Base::IsSceneExist(scene)) {
        // 同じ名前のシーンを削除
        // TODO log/ensure処理
        assert(
            0 &&
            "すでに同じタイプのシーンが用意されています。前のシーンは「無視」を押すことで削除して進めることができます");
        Base::ReleaseScene(scene);
    }

    if(current_scene_ == nullptr) {
        current_scene_ = scene;
    }
    else {
        next_scene_       = scene;
        scene_change_next = true;
    }

    Object::ClearObjectNames();
}

//! 次のシーンに切り替える
void Scene::ChangeNextScene()
{
    bool found = false;
    for(auto scene : scenes_) {
        // 登録シーンの中にあるか?
        if(current_scene_ == scene.second) {
            found = true;
            break;
        }
    }

    current_scene_ = next_scene_;
    if(!found) {
        // objectが解放できていない?
        ObjectWeakPtrVec objs = leak_objs;
        leak_objs.clear();
        for(auto obj : objs) {
            if(auto o = obj.lock()) {
                // コンポーネントをとらえてシーン移動しようとしている
                bool leak = false;
                for(auto weak_cmp : o->leak_components_) {
                    if(auto cmp = weak_cmp.lock())
                        leak = true;
                }

                if(leak) {
                    std::string str =
                        "オブジェクト名: " + std::string(o->GetName()) +
                        "\nオブジェクト内にてComponentPtrなどの変数で\nコンポーネントを確保したままになってないか確認してください.\n解決方法は、\n\n1. Objectの変数としない\n2. Exit()でnullptrにする\n3. weak_ptrに変更する\n\nいずれかを行うことで解決します.";
                    MessageBox(GetMainWindowHandle(), str.c_str(), "Objectが正しく解放できませんでした.", MB_OK);
                }
                // リークとしてとらえておく
                leak_objs.push_back(obj);
            }
        }
        // 終了の際は、確実にWindowを閉じます
        if(current_scene_ == nullptr) {
            DestroyWindow(GetMainWindowHandle());
        }
    }

    next_scene_       = nullptr;
    scene_change_next = false;

    scene_pause = false;   // ポーズ解除
}

//! 現在アクティブなシーンを取得します
Scene::Base* Scene::GetCurrentScene()
{
    return current_scene_.get();
}

//! 初期化処理
void Scene::Init()
{
    // ここではObjはInitしない
    // Worldが出来上がり、その後、
    // 個別にオブジェクトは作成されるため
    // Update内で、Initする形をとる
}

void Scene::Change(BasePtr scene)
{
    // すでに存在している?
    if(scene && Base::IsSceneExist(scene)) {
        // とらえているシーンに切り替える
        SetNextScene(scenes_[scene->Name()]);
        return;
    }

    SetNextScene(scene);
}

void Scene::PreUpdate()
{
    if(IsKeyOn(KEY_INPUT_F1))
        scene_pause = !scene_pause;

    if(IsKeyOn(KEY_INPUT_F2))
        scene_step = true;

    if(next_scene_ != nullptr || scene_change_next) {
        // シーン切り替え
        current_scene_->Exit();

        // 継承し忘れの時のため
        Scene::Exit();

        //ChangeNextScene();
        scene_time = 0.0f;
    }

    if(current_scene_) {
        if(!current_scene_->GetStatus(Scene::Base::StatusBit::Initialized)) {
            // ログ
            bool initialized = current_scene_->Init();
            current_scene_->SetStatus(Scene::Base::StatusBit::Initialized, initialized);

            // シーン初期化でブロッキングロードをしていた場合に時間経過が大きくなるためリセット
            ResetDeltaTime();
        }
        // シーン初期化できてない場合は Objectも初期化しない
        if(!current_scene_->GetStatus(Scene::Base::StatusBit::Initialized))
            return;

        // オブジェクト仮登録しているものを本登録に変更
        for(auto obj : current_scene_->pre_objects_) {
            auto& proc_update = obj->GetProc<float>(GetProcTimingName(ProcTiming::Update), ProcTiming::Update);
            auto& proc_draw   = obj->GetProc<void>(GetProcTimingName(ProcTiming::Draw), ProcTiming::Draw);

            current_scene_->Register(obj, proc_update.priority_, proc_draw.priority_);
        }
        // 仮登録のクリア
        current_scene_->pre_objects_.clear();

        if(!current_scene_->GetStatus(Scene::Base::StatusBit::Serialized)) {
            current_scene_->InitSerialize();
            current_scene_->SetStatus(Scene::Base::StatusBit::Serialized, true);
        }

        // 処理優先とポーズのチェック
        for(auto& obj : current_scene_->GetObjectPtrVec()) {
            // ポーズ中
            bool is_pause = false;
            if(obj->GetStatus(Object::StatusBit::IsPause) ||
               ((scene_pause && !scene_step) && !obj->GetStatus(Object::StatusBit::DisablePause)))
                is_pause = true;

            if(!is_pause) {
                // Init前状態
                if(!obj->GetStatus(Object::StatusBit::Initialized)) {
                    bool ret = obj->Init();
                    if(!ret)
                        continue;   //!< 初期化未終了

                    assert("継承先のInit()にて__super::Init()を入れてください." &&
                           obj->GetStatus(Object::StatusBit::Initialized));
                }

                functionSerialize(obj);
            }
#if 1
            // dirtyのチェック
            for(auto& timing : obj->update_timings_) {
                auto& proc = timing.second;
                if(proc.IsDirty()) {
                    current_scene_->resetProc(obj, proc);
                    current_scene_->setProc(obj, proc);
                    proc.ResetDirty();
                }
            }
            for(auto& timing : obj->proc_timings_) {
                auto& proc = timing.second;
                if(proc.IsDirty()) {
                    current_scene_->resetProc(obj, proc);
                    current_scene_->setProc(obj, proc);
                    proc.ResetDirty();
                }
            }

            for(auto& comp : obj->components_) {
                // Componentのdirtyのチェック
                for(auto& timing : comp->update_timings_) {
                    auto& proc = timing.second;
                    if(proc.IsDirty()) {
                        current_scene_->resetProc(comp, proc);
                        current_scene_->setProc(comp, proc);
                        proc.ResetDirty();
                    }
                }
                for(auto& timing : comp->proc_timings_) {
                    auto& proc = timing.second;
                    if(proc.IsDirty()) {
                        current_scene_->resetProc(comp, proc);
                        current_scene_->setProc(comp, proc);
                        proc.ResetDirty();
                    }
                }
            }
#endif

#if 1
            // オブジェクトのUpdate
            // TODO is_pause または NoUpdateが変化したときのみ処理をする
            if(obj->GetStatus(Object::StatusBit::NoUpdate) || is_pause) {
                for(auto& sig : obj->update_timings_) {
                    sig.second.connect_.block();
                }
                for(auto& sig : obj->proc_timings_) {
                    if((int)sig.second.IsUpdate())
                        sig.second.connect_.block();
                }

                // コンポーネントのUpdate
                for(auto& component : obj->GetComponents()) {
                    for(auto& sig : component->update_timings_) {
                        sig.second.connect_.block();
                    }
                    for(auto& sig : component->proc_timings_) {
                        if((int)sig.second.IsUpdate())
                            sig.second.connect_.block();
                    }
                }
            }
            else {
                for(auto& sig : obj->update_timings_) {
                    sig.second.connect_.unblock();
                }
                for(auto& sig : obj->proc_timings_) {
                    if((int)sig.second.IsUpdate())
                        sig.second.connect_.unblock();
                }

                // コンポーネントのUpdate
                for(auto& component : obj->GetComponents()) {
                    for(auto& sig : component->update_timings_) {
                        sig.second.connect_.unblock();
                    }
                    for(auto& sig : component->proc_timings_) {
                        if((int)sig.second.IsUpdate())
                            sig.second.connect_.unblock();
                    }
                }
            }
#endif

#if 1
            // オブジェクトのDraw
            if(obj->GetStatus(Object::StatusBit::NoDraw)) {
                for(auto& sig : obj->proc_timings_) {
                    if((int)sig.second.IsDraw())
                        sig.second.connect_.block();
                }

                // コンポーネント
                for(auto& component : obj->GetComponents()) {
                    for(auto& sig : component->proc_timings_) {
                        if((int)sig.second.IsDraw())
                            sig.second.connect_.block();
                    }
                }
            }
            else {
                for(auto& sig : obj->proc_timings_) {
                    if((int)sig.second.IsDraw())
                        sig.second.connect_.unblock();
                }

                // コンポーネント
                for(auto& component : obj->GetComponents()) {
                    for(auto& sig : component->proc_timings_) {
                        if((int)sig.second.IsDraw())
                            sig.second.connect_.unblock();
                    }
                }
            }
#endif
        }
        current_scene_->GetSignals(ProcTiming::PreUpdate)();
    }
}

//! 更新処理
void Scene::Update(float delta)
{
    if(current_scene_) {
        if(!scene_pause || scene_step) {
            current_scene_->Update(delta);
            scene_time += delta;
        }

        current_scene_->GetUpdateSignals(ProcTiming::Update)(delta);
        current_scene_->GetUpdateSignals(ProcTiming::LateUpdate)(delta);
    }
}

void Scene::PrePhysics()
{
    if(current_scene_) {
        current_scene_->GetSignals(ProcTiming::PrePhysics)();

        // Physics
        CheckComponentCollisions();
    }
}

void Scene::PostUpdate()
{
    if(current_scene_)
        current_scene_->GetSignals(ProcTiming::PostUpdate)();
}

void Scene::Draw()
{
    // シーンが無ければ何もしない
    if(!current_scene_)
        return;

    // シーンDrawの実行
    current_scene_->Draw();
    scene_step = false;

    // プロセスシグナルの実行
    current_scene_->GetSignals(ProcTiming::PreDraw)();

    current_scene_->GetSignals(ProcTiming::Shadow)();
    current_scene_->GetSignals(ProcTiming::Gbuffer)();
    current_scene_->GetSignals(ProcTiming::Light)();
    current_scene_->GetSignals(ProcTiming::HDR)();

    current_scene_->GetSignals(ProcTiming::Draw)();
    current_scene_->GetSignals(ProcTiming::LateDraw)();
    current_scene_->GetSignals(ProcTiming::PostDraw)();

    // 未使用のコンポーネントを削除
    for(auto obj : current_scene_->GetObjectPtrVec()) {
        obj->ModifyComponents();
    }

    // 終了した場合( Exit()を呼んだ場合 )
    for(int i = (int)current_scene_->GetObjectPtrVec().size() - 1; i >= 0; --i) {
        auto obj = current_scene_->GetObjectPtrVec()[i];
        if(!obj->GetStatus(Object::StatusBit::Alive)) {
            if(!obj->GetStatus(Object::StatusBit::Exited))
                obj->Exit();
        }
        if(obj->GetStatus(Object::StatusBit::Exited)) {
            current_scene_->Unregister(obj);
            continue;
        }
    }

    // PauseSystem::DrawPause();
}

void Scene::Exit()
{
    if(current_scene_) {
        for(auto& obj : current_scene_->GetObjectPtrVec()) {
            if(!obj->GetStatus(Object::StatusBit::Exited)) {
                obj->Exit();
            }
        }
        current_scene_->Exit();
        current_scene_->UnregisterAll();
    }
#if 0
    physx_lib->Exit();
#endif
    //	Change( nullptr );
    ChangeNextScene();
}

void Scene::GUI()
{
    // オブジェクトPICK
    if(IsMouseOn(MOUSE_INPUT_LEFT) && !ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow) && !ImGuizmo::IsOver() &&
       !IsShowMenu()) {
        auto   obj  = PickObject(GetMouseX(), GetMouseY());
        auto   objs = current_scene_->GetObjectPtrVec();
        size_t max  = objs.size();

        for(int i = 0; i < max; i++) {
            if(obj == objs[i]) {
                select_object_index = i;
                selectObject        = current_scene_->GetObjectPtrVec()[select_object_index];
            }
        }
    }

    // エディター状態でなければGUIは出ない
    if(!scene_edit)
        return;

    // シーンが無ければ何もしない
    if(!current_scene_)
        return;

    ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.0f, 0.7f, 0.2f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.0f, 0.3f, 0.1f, 1.0f));

    // GUIウインドウ設定
    SetGUIWindow(SETGUI_ATTACH_RIGHT, 0, (int)inspector_size.x, (int)inspector_size.y);

    // シーンのGUIを表示
    std::string name = "Scene : " + current_scene_->Name();

    auto flags = ImGuiWindowFlags_None;
    if(GetEditorStatus(EditorStatusBit::EditorPlacement))
        flags = ImGuiWindowFlags_NoCollapse;

    ImGui::Begin(name.c_str(), (bool*)nullptr, flags);
    {
        if(ImGui::TreeNode(u8"登録シーン")) {
            ImGui::Text(u8"シーン数 : %d", GetSceneCount());
            ImGui::Text(u8"TODO: 存在シーン列挙");
            ImGui::Text(u8"全Object数 : %d", Object::ExistObjectCount());

            if(leak_objs.size() > 0)
                ImGui::TextColored({1, 0, 0, 1}, u8"LeakObject数 : %d", leak_objs.size());
            ImGui::TreePop();
        }

        ImGui::DragFloat(u8"経過時間", &scene_time, 0.1f, 0, 0, "%.2f");
        ImGui::Text(u8"シーン内Object数 : %d", current_scene_->GetObjectPtrVec().size());

        std::vector<const char*> listbox;
        listbox.reserve(current_scene_->GetObjectPtrVec().size());
        for(auto& obj : current_scene_->GetObjectPtrVec()) {
            listbox.emplace_back(obj->GetName().data());
        }

        int size = (int)current_scene_->GetObjectPtrVec().size();

        //  シーン オブジェクトインスペクタ
        auto xy = ImGui::GetWindowSize();
        ImGui::BeginChild(ImGui::GetID((void*)Scene::GUI), ImVec2(xy.x - 40, xy.y - 100), ImGuiWindowFlags_NoTitleBar);
        {
            for(int i = 0; i < size; i++) {
                auto obj = current_scene_->GetObjectPtrVec()[i];
                if(!Scene::GetEditorStatus(EditorStatusBit::EditorPlacement)) {
                    ImGui::CheckboxFlags(std::to_string(i).c_str(),
                                         (int*)&obj->status_,
                                         1 << (int)Object::StatusBit::ShowGUI);
                    ImGui::SameLine();
                }
                if(ImGui::Selectable(obj->GetName().data(), select_object_index == i)) {
                    select_object_index = i;
                    selectObject        = current_scene_->GetObjectPtrVec()[select_object_index];
                }
            }
            for(int i = 0; i < leak_objs.size(); i++) {
                auto weak_obj = leak_objs[i];
                auto obj      = weak_obj.lock();
                if(obj == nullptr)
                    continue;

                if(!Scene::GetEditorStatus(EditorStatusBit::EditorPlacement)) {
                    ImGui::CheckboxFlags(std::to_string(i).c_str(),
                                         (int*)&obj->status_,
                                         1 << (int)Object::StatusBit::ShowGUI);
                    ImGui::SameLine();
                }
                if(ImGui::Selectable(obj->GetName().data(),
                                     select_object_index == i + size,
                                     ImGuiSelectableFlags_::ImGuiSelectableFlags_Disabled)) {
                    select_object_index = i + size;
                    selectObject        = leak_objs[select_object_index - size];
                }
            }
        }
        ImGui::EndChild();
        /*
		ImGui::ListBox( u8"シーン内オブジェクト", &select_object_index, listbox.data(), (int)listbox.size(), size );
		if( select_object_index != -1 )
		{
			selectObject = current_scene_->GetObjectPtrVec()[ select_object_index ];
			auto obj	 = selectObject.lock();
			bool b		 = !obj->GetStatus( Object::StatusBit::ShowGUI );
			obj->SetStatus( Object::StatusBit::ShowGUI, b );
		}
		*/

        // シーンのGUI描画を実行
        current_scene_->GUI();

        auto ims       = ImGui::GetWindowSize();
        inspector_size = {ims.x, ims.y};
    }
    ImGui::End();

    ImGui::PopStyleColor();
    ImGui::PopStyleColor();

    size_t size = current_scene_->GetObjectPtrVec().size();
    for(int i = 0; i < size; i++) {
        auto obj = current_scene_->GetObjectPtrVec()[i];
        if(obj->GetStatus(Object::StatusBit::ShowGUI)) {
            // GUIウインドウ設定
            if(Scene::GetEditorStatus(Scene::EditorStatusBit::EditorPlacement)) {
                if(i != select_object_index) {
                    continue;
                }
                Scene::SetGUIWindow(Scene::SETGUI_ATTACH_RIGHT,
                                    (int)inspector_size.y,
                                    (int)object_detail_size.x,
                                    (int)object_detail_size.y);
            }

            obj->SetStatus(Object::StatusBit::CalledGUI, false);
            obj->GUI();
            assert("継承先のGUI()にて__super::GUI()を入れてください." && obj->GetStatus(Object::StatusBit::CalledGUI));

            for(auto component : obj->GetComponents()) {
                if(component->GetStatus(Component::StatusBit::ShowGUI))
                    component->GUI();
#ifdef _DEBUG
//          obj->Checker();
#endif   //_DEBUG
            }
        }
    }
}

bool Scene::IsPause()
{
    return scene_pause;
}

float Scene::GetTime()
{
    return scene_time;
}

void Scene::SetEdit(bool edit)
{
    scene_edit = edit;
}

const bool Scene::IsEdit()
{
    return scene_edit;
}

//! @brief ComponentCollisionの当たり判定を行う
void Scene::CheckComponentCollisions()
{
    // オブジェの数を取得
    int obj_num = (int)current_scene_->GetObjectPtrVec().size();
    for(int obj_index = 0; obj_index < obj_num; obj_index++) {
        // 調べるオブジェクト
        auto obj = current_scene_->GetObjectPtrVec()[obj_index];

        // コンポーネントコリジョン群の取得
        auto cols = obj->GetComponents<ComponentCollision>();
        {
            // コリジョンの検査
            for(auto col_1 : cols) {
                // 相手オブジェクトの取得
                for(int other_index = obj_index + 1; other_index < obj_num; other_index++) {
                    std::vector<ComponentCollision::HitInfo> hitInfos;

                    // 相手オブジェクト
                    auto obj2 = current_scene_->GetObjectPtrVec()[other_index];

                    auto cols2 = obj2->GetComponents<ComponentCollision>();

                    // 相手コリジョンの検査
                    for(auto col_2 : cols2) {
                        if(!col_1->IsGroupHit(col_2))
                            continue;

                        //! @todo オブジェクトの当たりをチェックする
                        ComponentCollision::HitInfo hitInfo;
                        hitInfo = col_1->IsHit(col_2);

                        if(hitInfo.hit_) {
                            // 押し戻し量再計算
                            float3 push{hitInfo.push_ * 0.5f};
                            float3 other_push{-hitInfo.push_ * 0.5f};
                            col_1->CalcPush(col_2, hitInfo.push_, &push, &other_push);

                            hitInfo.collision_     = col_1;
                            hitInfo.hit_collision_ = col_2;
                            hitInfo.push_          = push;
                            col_1->OnHit(hitInfo);

                            hitInfo.collision_     = col_2;
                            hitInfo.hit_collision_ = col_1;
                            hitInfo.push_          = other_push;
                            col_2->OnHit(hitInfo);
                        }
                    }
                }
            }
        }
    }
}

//! セレクトしているオブジェクトかをチェックする
bool Scene::SelectObjectWindow(const ObjectPtr& object)
{
    auto obj = selectObject.lock();
    if(obj != nullptr) {
        auto sel_name = obj->GetName();
        auto my_name  = object->GetName();
        if(object->GetName() == obj->GetName()) {
            selectObject.reset();
            return true;
        }
    }
    return false;
}

void Scene::SetGUIObjectDetailSize()
{
    auto ims           = ImGui::GetWindowSize();
    object_detail_size = {ims.x, ims.y};
}

void Scene::SetGUIWindow(int posx, int posy, int width, int height)
{
    if(Scene::GetEditorStatus(Scene::EditorStatusBit::EditorPlacement)) {
        auto mode = ImGuiCond_Once;
        if(Scene::GetEditorStatus(Scene::EditorStatusBit::EditorPlacement_Always))
            mode = ImGuiCond_Always;

        int game_width = 0, game_height = 0;
        int game_posx = 0, game_posy = 0;
        GetWindowSize(&game_width, &game_height);
        GetWindowPosition(&game_posx, &game_posy);

        if(posx == SETGUI_ATTACH_LEFT) {
            posx = game_posx - width;
        }
        else if(posx == SETGUI_ATTACH_RIGHT) {
            posx = game_posx + game_width + 8;
        }
        else {
            posx += game_posx;
        }

        if(posy == SETGUI_ATTACH_TOP) {
            posy = game_posy - height;
        }
        else if(posy == SETGUI_ATTACH_BOTTOM) {
            posy = game_posy + game_height;
        }
        else {
            posy += game_posy;
        }

        ImGui::SetNextWindowPos(ImVec2((float)posx, (float)posy), mode);
        ImGui::SetNextWindowSize(ImVec2((float)width, (float)height), mode);
    }
}

ObjectPtr Scene::PickObject(int x, int y)
{
    auto wcam = GetCurrentCamera();
    auto cam  = wcam.lock();
    if(cam == nullptr)
        return nullptr;

    ComponentCamera::CameraRay ray = cam->MousePositionRay(x, y);

    auto vec = normalize(ray.end - ray.start) * 5.0f;

    DrawCapsule3D(cast(ray.start + vec), cast(ray.end), 0.05f, 30, GetColor(255, 0, 0), GetColor(255, 0, 0), TRUE);

    float     min     = FLT_MAX;
    ObjectPtr min_obj = nullptr;

    for(auto& obj : current_scene_->GetObjectPtrVec()) {
        auto mdl = obj->GetComponent<ComponentModel>();
        if(mdl) {
            MV1SetupCollInfo(mdl->GetModel(), -1, 8, 8, 8);
            MV1_COLL_RESULT_POLY result = MV1CollCheck_Line(mdl->GetModel(), -1, cast(ray.start), cast(ray.end));
            if(result.HitFlag) {
                float near_len = length(cast(result.HitPosition) - cam->GetPosition());
                if(near_len < min)
                    min_obj = obj;
            }
        }
    }
    return min_obj;
}

ComponentCameraWeakPtr Scene::GetCurrentCamera()
{
    // デバック中であればデバックカメラを返す
    if(DebugCamera::IsUse())
        return DebugCamera::GetCamera();

    // 通常のカレントカメラを返す
    return ComponentCamera::GetCurrentCamera();
}

ComponentCameraWeakPtr Scene::SetCurrentCamera(const std::string_view name)
{
    if(auto obj = GetObjectPtr<Object>(name)) {
        if(auto cam = obj->GetComponent<ComponentCamera>())
            cam->SetCurrentCamera();
    }

    // デバック中であればデバックカメラを返す
    if(DebugCamera::IsUse())
        return DebugCamera::GetCamera();

    // 通常のカレントカメラを返す
    return ComponentCamera::GetCurrentCamera();
}

//----------------------------------------------------------------------------
// Doxygen Sceneマニュアル
//----------------------------------------------------------------------------
//! @page section_scene Sceneについて
//!	@li スマートポインタにて管理 (オブジェクト管理が楽)
//!	@li シーン切り替えが容易にできる
//!	@li GUIにて現状把握できる
//!
//!	@li 【簡易Scene使用方法】①シーンを用意する
//!	@code
//!	class SceneSample : public Scene::Base
//!	{
//!	public:
//!		std::string Name() override {   //< シーンに名前をつける必要があります
//!			return "Sample";            //< 別のシーンに同じ名前はを使用しないでください。
//!		}                               //< SceneXXXXのXXXXを名前にするとよいと思います。
//!
//!		bool Init() override
//!		{
//!			// Objectの初期化例
//!			auto obj = Scene::CreateObject<Object>();
//!			obj->AddComponent<ModelComponent>("Model.mv1");
//!			obj->AddComponent<CameraComponent>();
//!		}
//!		void Update(float delta) override
//!		{
//!			obj->Update(delta);
//!
//!			// シーンが始まって10秒以上たったらSceneNextへ
//!			if (Scene::GetTime() > 10.0f)
//!			   Scene::Change( Scene::GetScene<SceneNext>() );
//!		}
//!		void Draw() override
//!		{
//!			obj->Draw();
//!		}
//!		void Exit() override
//!		{
//!		}
//!		void GUI() override
//!		{
//!
//!		}
//!	};
//!	@endcode
//!
//!	@li 【簡易Scene使用方法】②シーンを切り替える (初期はGameMainで切り替えます)
//!	@code
//!	// 例: シーンを「SceneSample」に変更します
//!	  Scene::Change( Scene::GetScene<SceneSample>() );
//!
//!	@endcode
