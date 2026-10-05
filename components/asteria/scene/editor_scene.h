#pragma once
#include "scene/scene.h"
namespace asteria {
void editor_set_map(int m);
struct EditorScene : Scene {
  void enter() override; void update(SceneManager&) override; void render() override;
};
EditorScene& editor_scene();
}
