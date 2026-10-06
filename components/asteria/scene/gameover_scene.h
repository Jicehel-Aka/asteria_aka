#pragma once
#include "scene/scene.h"
#include <cstdint>
namespace asteria {
struct GameOverScene : Scene {
  void enter() override; void update(SceneManager&) override; void render() override;
  uint32_t t0=0;
};
GameOverScene& gameover_scene();
}
