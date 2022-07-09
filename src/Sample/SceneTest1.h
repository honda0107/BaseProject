#pragma once

#include <System/Scene.h>

class SceneTest1 : public Scene::Base
{
public:
    BP_CLASS_TYPE(SceneTest1, Scene::Base);

    std::string Name() override { return "Test1"; }

    bool Init() override;
    void Update(float delta) override;
    void Draw() override;
    void Exit() override;
    void GUI() override;

    void InitSerialize() override;
};
