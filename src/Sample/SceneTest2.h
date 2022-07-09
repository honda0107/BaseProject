#pragma once

#include <System/Scene.h>

class SceneTest2 : public Scene::Base
{
public:
    BP_CLASS_TYPE(SceneTest2, Scene::Base);

    std::string Name() override { return "Test2"; }

    bool Init() override;
    void Update(float delta) override;
    void Draw() override;
    void Exit() override;
    void GUI() override;
};
