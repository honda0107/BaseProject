#pragma once

#include <System/Scene.h>

class SceneTestCollision : public Scene::Base
{
public:
    BP_CLASS_TYPE(SceneTestCollision, Scene::Base);

    std::string Name() override { return "TestCollision"; }

    bool Init() override;
    void Update(float delta) override;
    void Draw() override;
    void Exit() override;
    void GUI() override;
};
