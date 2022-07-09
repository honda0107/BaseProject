#pragma once

#include <System/Scene.h>

//! @brief シーン
//! @detail Scene::Baseより継承させ Scene○○と名前をつけておく。
//! @detail 今回はObjectを使用する説明のためSceneTestObjectと名前をつけています
//! シーンは、タイトル、ゲーム中、ゲームオーバーなどのシーンを用意し、切り替えて使えます
class SceneTestObject : public Scene::Base
{
public:
    // 継承している形をここでも書いておく(プログラム的に使用する)
    BP_CLASS_TYPE(SceneTestObject, Scene::Base);

    // シーン名前取得関数は必ず継承しておく必要がある
    std::string Name() override
    {
        // Sceneを省いた名前をお勧めします
        return "TestObject";
    }

    //! @brief シーン初期化関数を継承します
    //! @return シーン初期化が終わったらtrueを返します
    bool Init() override;

    //! @brief シーン更新関数。ディスプレイリフレッシュレートに合わせて実行されます
    //! @param delta 1秒をベースとした1フレームの数値
    //! @detial deltaは、リフレッシュレートが違うと速度が変わってしまう部分を吸収するためにある
    void Update(float delta) override;

    //! @brief 終了処理
    void Exit() override;
};
