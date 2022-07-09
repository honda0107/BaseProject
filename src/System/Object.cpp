//---------------------------------------------------------------------------
//! @file   Object.cpp
//! @brief  オブジェクト ベースクラス
//---------------------------------------------------------------------------
#include "Object.h"
#include <System/Component/ComponentTransform.h>
#include <System/Scene.h>

#include <unordered_map>
#include <string>

namespace
{
std::unordered_map<std::string, u64> obj_names_id;      //!< 存在するオブジェクト名
std::unordered_map<std::string, u64> obj_names_count;   //!< 存在するオブジェクト名

size_t obj_count = 0;

}   // namespace

//! @brief 名前から、ユニークな名前を設定します
//! @param name つけたい名前
//! @return 通常はそのまま返しますが、同じ名前がある場合は違う名前に変更されます
std::string Object::setUniqueName(const std::string& name)
{
    auto itr = obj_names_id.find(name);

    if(itr != obj_names_id.end()) {
        // 登録済なら連番で別名をつける
        auto str = name + "_" + std::to_string(obj_names_id[name]);
        obj_names_id[name]++;
        obj_names_count[name]++;
        return str;
    }
    // 登録済の名前がなかった場合は新規登録
    obj_names_id[name]++;
    obj_names_count[name]++;
    return name;
}

Object::Object()
{
    SetName("object", true);
    obj_count++;
    SetStatus(StatusBit::Alive, true);
}

Object::~Object()
{
    obj_count--;

    std::string str = "~Object:" + std::string(GetName()) + "\n";
    OutputDebugString(str.c_str());

    // すでにシーンが終了している?
    if(obj_names_count.empty())
        return;

    // この名前のものがほかにいないか?
    auto name  = std::string(GetNameDefault());
    u64  count = --obj_names_count[name];
    if(count == 0)
        obj_names_id.erase(name);
}

//! @brief 初期化処理
//! @retval true : 初期化が終わっている
//! @retval false : 初期化未完了(別スレッドロードなどで)
bool Object::Init()
{
    SetStatus(StatusBit::ShowGUI, true);
    SetStatus(StatusBit::Initialized, true);
    SetStatus(StatusBit::Serialized, false);
    return true;
}

//! @brief 更新
//! @param delta 更新秒数
void Object::Update([[maybe_unused]] float delta)
{
}

//! @brief 遅い更新
//! @param delta 更新秒数
void Object::LateUpdate([[maybe_unused]] float delta)
{
}

//! @brief 描画
void Object::Draw()
{
}

//! @brief 描画
void Object::LateDraw()
{
}

//! @brief 終了
void Object::Exit()
{
    SetStatus(Object::StatusBit::Exited, true);
}

//! @brief GUI処理
void Object::GUI()
{
    // GUI処理
    ImGui::Begin(name_.c_str());
    {
        //　シーンで選択されている場合はフォーカスする
        if(Scene::SelectObjectWindow(shared_from_this()))
            ImGui::SetKeyboardFocusHere(-1);

        // 使用しているコンポーネント数
        ImGui::Text(u8"使用コンポーネント数: %d", components_.size());

        // 状態制御
        if(ImGui::TreeNode(u8"状態(Status)")) {
            // Updateしない
            bool noUpdate = GetStatus(StatusBit::NoUpdate);
            // Drawしない
            bool noDraw = GetStatus(StatusBit::NoDraw);
            // Pauseしない
            bool disPause = GetStatus(StatusBit::DisablePause);
            // ポーズ中(ゲームポーズとは別)
            bool isPause = GetStatus(StatusBit::IsPause);

            ImGui::Checkbox(u8"更新OFF", &noUpdate);
            ImGui::Checkbox(u8"描画OFF", &noDraw);
            ImGui::Checkbox(u8"PAUSE禁止", &disPause);
            ImGui::Checkbox(u8"PAUSE", &isPause);

            SetStatus(StatusBit::NoUpdate, noUpdate);
            SetStatus(StatusBit::NoDraw, noDraw);
            SetStatus(StatusBit::DisablePause, disPause);
            SetStatus(StatusBit::IsPause, isPause);

            ImGui::Separator();
            ImGui::TreePop();
        }
        Scene::SetGUIObjectDetailSize();
    }
    ImGui::End();

    SetStatus(StatusBit::CalledGUI, true);
}

//! @brief 更新前処理
void Object::PreUpdate()
{
    //初期配置されていない場合はワープさせる
    if(!GetStatus(StatusBit::Located)) {
        if(auto trns = GetComponent<ComponentTransform>())
            trns->PostUpdate();
    }
}

//! @brief 更新後処理
void Object::PostUpdate()
{
    SetStatus(StatusBit::Located, true);
}

//! @brief 描画前処理
void Object::PreDraw()
{
}

//! @brief 描画後処理
void Object::PostDraw()
{
}

//! @brief 物理シミュレーション前処理
void Object::PrePhysics()
{
#ifdef USE_JOLT_PHYSICS
#else
    if(GetComponent<ComponentTransform>()) {
        float3 g = gravity_;
        Matrix()._41_42_43 += gravity_;
        gravity_ = 0.0f;
    }
#endif
}

//! シリアライズでもどらないユーザー処理関数などを設定
void Object::InitSerialize()
{
    SetStatus(StatusBit::Serialized, true);
}

//! @brief ステータスの設定
//! @param b セットするビット
//! @param on 状態
void Object::SetStatus(StatusBit b, bool on)
{
    on ? status_.on(b) : status_.off(b);
}

//! @brief ステータス取得
//! @param b 取得するビット
//! @return 状態
bool Object::GetStatus(StatusBit b)
{
    return status_.is(b);
}

//! @brief 名前の取得
//! @return 名前(string_view)
std::string_view Object::GetName() const
{
    return name_;
}

//! @brief 番号なしの名前の取得 (オブジェクトを消す際に使える)
//! @return 名前(string_view)
std::string_view Object::GetNameDefault() const
{
    return name_default_;
}

//! @brief 実際にObject newされ、deleteしてない数
//! @return 存在するObject
size_t Object::ExistObjectCount()
{
    return obj_count;
}

//! @brief コンポーネントの削除チェック
void Object::ModifyComponents()
{
    auto& components = GetComponents();
    for(int i = (int)components.size() - 1; i >= 0; --i) {
        auto& c = components[i];
        if(c->status_.is(Component::StatusBit::Exited)) {
            auto comp = components.begin() + i;

            ComponentWeakPtr weak_comp = *comp;
            components.erase(comp);   //解放処理(自動delete)

            if(weak_comp.lock() != nullptr) {
                // どこかに残っているので一旦確保
                leak_components_.push_back(weak_comp);
            }
        }
    }
}

//! コンポーネント削除
//! @param [in] component 削除するコンポーネント
void Object::RemoveComponent(ComponentPtr component)
{
    for(int i = (int)components_.size() - 1; i >= 0; --i) {
        auto& c = components_[i];

        if(c == component) {
            c->Exit();
            break;
        }
    }
}

//! @brief コンポーネントをすべて削除する
void Object::RemoveAllComponents()
{
    for(int i = (int)components_.size() - 1; i >= 0; --i) {
        auto& c = components_[i];
        c->Exit();
        assert("__super::Exit() を継承先の Exit()　にて使用してください." &&
               c->status_.is(Component::StatusBit::Exited));
    }
}

void Object::RemoveAllProcesses()
{
    for(auto& t : update_timings_) {
        auto& p = t.second;
        if(p.connect_.valid())
            p.connect_.disconnect();

        p.proc_ = nullptr;
    }
    update_timings_.clear();

    for(auto& t : proc_timings_) {
        auto& p = t.second;
        if(p.connect_.valid())
            p.connect_.disconnect();

        p.proc_ = nullptr;
    }
    proc_timings_.clear();

    for(auto& cmp : this->components_) {
        for(auto& t : cmp->update_timings_) {
            auto& p = t.second;
            if(p.connect_.valid())
                p.connect_.disconnect();

            p.proc_ = nullptr;
        }
        cmp->update_timings_.clear();

        for(auto& t : cmp->proc_timings_) {
            auto& p = t.second;
            if(p.connect_.valid())
                p.connect_.disconnect();

            p.proc_ = nullptr;
        }
        cmp->proc_timings_.clear();
    }
}

//! @brief   使用している名前リストを消去する
//! @details ステージ移行などでオブジェクトが消去されたときに呼ぶ
void Object::ClearObjectNames()
{
    obj_names_id.clear();
    obj_names_count.clear();
}

//@}
//----------------------------------------------------------------------
//! @name ComponentTransform関係
//----------------------------------------------------------------------
//@{

//! @brief TransformのMatrix情報を取得します
//! @return ComponentTransform の Matrix
matrix& Object::Matrix()
{
    auto cmp = GetComponent<ComponentTransform>();

    assert(cmp && "このオブジェクトは、ComponentTransformが存在していません。位置移動はできません");

    return cmp->Matrix();
}

//! @brief ワールドMatrixの取得
//! @return 他のコンポーネントも含めた位置
const matrix Object::GetOldWorldMatrix()
{
    auto cmp = GetComponent<ComponentTransform>();

    assert(cmp && "このオブジェクトは、ComponentTransformが存在していません。位置移動はできません");

    return cmp->GetOldWorldMatrix();
}

//----------------------------------------------------------------------------------------
// Doxygenマニュアル
//----------------------------------------------------------------------------------------
//! @page section_object Objectについて
//! @li シーン内にて使用することが可能 (シーンがなくなるとObjectも消えます)
//! @li コンポーネント(Component)を追加できる
//! @li 名前設定が可能でGUIで情報を見ることが可能
//!
//! @li // 生成方法【例】(シーン内にて)
//! @code
//! 　
//!     auto obj = Scene::CreateObject<Object>();
//!     auto item = Scene::CreateObject<Item>();    //< ItemはObjectから継承したクラス
//!
//!     // 名前を変えることも可能です。(GUI上でも名前が変わります)
//!     item->SetName("WhiteItem");
//!
//!     Scene::CreateObject<Player>()->SetName("RedPlayer"); //< 変数に捉えない場合。取得方法にて取得する。
//! 　
//! @endcode
//!
//! @li // 取得方法【例】(シーン内にて)
//! @code
//! 　
//!     // 存在しているPlayerのオブジェクトを取得します
//!     auto player = Scene::GetObjectPtr<Player>();   //< Playerクラスが一つしかない場合はこれでOK
//!
//!     auto red_player = Scene::GetObjectPtr<Player>("RedPlayer");   //< 複数の時は名前も指定しないと別のものが取れる可能性がある
//! 　
//! @endcode
//!
//! @li // 削除方法【例】(シーン内にて)
//! @code
//! 　
//!     // 存在しているオブジェクトを削除します
//!
//!     Scene::ReleaseObject<Player>();   //< Playerクラスが一つしかない場合はこれでOK
//! 　
//!		Scene::ReleaseObject("RedPlayer"); //< 名前指定で削除する場合
//!
//! @endcode
//!
//! @li // 継承方法【例】
//! @code
//! 　
//! class Player : public Object
//! {
//! public:
//!     bool Init() override
//!     {
//!         __super::Init();    //< Object内でInitが必要なものを定義します※忘れるとASSERTが出ます
//!
//!         // 処理化処理をかく
//!
//!         return true;        //< 初期化が終了したら trueで返してください (※現状機能不完全)
//!                             //  falseの場合は、もう一度Init()が呼ばれます。(ロード完了で次へ進むなどの場合を想定しています)
//!     }
//!
//!     void Update() override
//!     {
//!         // 更新処理を行う
//!     }
//!
//!     void Draw() override
//!     {
//!         // 描画処理を行う
//!     }
//!
//!     void Exit() override
//!     {
//!         __super::Exit();    //< Object内でExitの終了も呼びます　※忘れるとASSERTが出ます
//!         // 終了処理を行う
//!     }
//!
//!     void GUI() override
//!     {
//!         __super::GUI();     //<! オブジェクトのGUIを使えるようにしておく
//!
//!         // PlayerGUI
//!         ImGui::Begin(name_.c_str());
//!         {
//!             // GUI処理をここに追加する
//!         }
//!         ImGui::End()
//!     }
//! };
//! 　
//! @endcode
