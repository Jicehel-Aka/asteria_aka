#pragma once
#include "scene/scene.h"
#include "save.h"
namespace asteria {
struct TitleScene : Scene {
  int sel=0; bool has_save=false;
  int slot=1; SaveSummary sums[3]{};
  void enter() override; void update(SceneManager&) override; void render() override;
};
TitleScene& title_scene();
}
