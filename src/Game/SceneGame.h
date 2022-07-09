#pragma once

#include <System/Scene.h>

class SceneGame : public Scene::Base
{
public:
    BP_CLASS_TYPE(SceneGame, Scene::Base);

    std::string Name() override { return "SceneGame"; }

    bool Init() override;
    void Update(float delta) override;
    void Draw() override;
    void Exit() override;
    void GUI() override;
};
