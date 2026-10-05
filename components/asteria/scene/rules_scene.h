#pragma once
#include "scene/scene.h"
namespace asteria { struct RulesScene : Scene {
  int page=0;
  void enter() override; void update(SceneManager&) override; void render() override;
}; RulesScene& rules_scene(); }
