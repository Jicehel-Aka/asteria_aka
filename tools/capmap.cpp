// Capture d'ecran des cartes (host) : charge chaque carte, place le joueur, rend une frame PPM.
#include "scene/scene.h"
#include "scene/world_scene.h"
#include "platform/gb_port.h"
#include "player.h"
#include "gamestate.h"
#include "audio/jingle.h"
using namespace asteria;
void host_set_budget(int n); void host_inject(int code); void host_tick();
int main(){
  audio::init(); host_set_budget(1000000000); new_game_reset();
  SceneManager mgr; mgr.set(SceneId::WORLD);
  struct S{int m,x,y;}; S shots[]={ {1,16,14}, {1,22,24}, {5,11,10}, {5,11,42}, {8,12,11}, {8,17,8} };
  for(S s: shots){
    WorldScene& w=world_scene(); w.load_map(s.m); w.px=s.x; w.py=s.y; w.notice=false; w.dlg=false;
    host_inject(-1); mgr.update(); mgr.render(); gb::frame_end();
  }
  return 0;
}
