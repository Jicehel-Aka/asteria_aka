#pragma once
#include "scene/scene.h"
namespace asteria { struct TitleScene : Scene {
  int sel=0; bool has_save=false;
  void enter() override; void update(SceneManager&) override; void render() override;
}; TitleScene& title_scene(); }
