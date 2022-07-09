#pragma once

#include <System/Scene.h>

//! @brief シーン
class SceneObjectTracking : public Scene::Base
{
public:
    // 継承している形をここでも書いておく(プログラム的に使用する)
    BP_CLASS_TYPE(SceneObjectTracking, Scene::Base);

    // シーン名前取得関数は必ず継承しておく必要がある
    std::string Name() override
    {
        // Sceneを省いた名前をお勧めします
        return "ObjectTracking";
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
