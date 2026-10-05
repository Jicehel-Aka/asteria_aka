#pragma once
#include "scene/scene.h"
#include <cstdint>
namespace asteria { struct WorldScene : Scene {
  int map=0; int px=0, py=0; int dir=0; int hf=1; uint32_t lastMove=0; bool notice=false; const char* noticeTxt=nullptr; bool started=false; bool confirmQuit=false; bool dlg=false; int dlgNpc=-1; int dlgNode=0; int dlgLine=0;
  void enter() override; void update(SceneManager&) override; void render() override;
  void load_map(int m);
}; WorldScene& world_scene();
void new_game_reset(); }
