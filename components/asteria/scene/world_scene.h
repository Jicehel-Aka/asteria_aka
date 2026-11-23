#pragma once
#include "scene/scene.h"
#include <cstdint>
namespace asteria {
struct WorldScene : Scene {
  int map=0; int px=0, py=0; int dir=0; int hf=1; uint32_t lastMove=0;
  int camX=0, camY=0, camMap=-1;   // camera persistante (zone morte) ; camMap != map -> recadrage
  bool notice=false; const char* noticeTxt=nullptr; bool started=false;
  bool confirmQuit=false; bool dlg=false; int dlgNpc=-1; int dlgNode=0; int dlgLine=0;
  int savedMap=-2; int epiLine=0;
  int pauseSel=0; int pauseSub=0; int pauseListSel=0; int stepCount=0;
  const char* pauseMsg=nullptr;
  void enter() override; void update(SceneManager&) override; void render() override;
  void load_map(int m);
  void pause_update(SceneManager& mgr,uint32_t pressed);
};
WorldScene& world_scene();
void new_game_reset();
namespace data { struct Map; }
void draw_map_tile(const data::Map& M,int x,int y);
}
