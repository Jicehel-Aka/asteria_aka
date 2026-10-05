#pragma once
#include "scene/scene.h"
namespace asteria {
struct CreateScene : Scene {
  int step=0, selChoice=0, cur=0, st[4]={8,8,8,8}, pool=18;
  void enter() override; void update(SceneManager&) override; void render() override;
};
CreateScene& create_scene();
}
