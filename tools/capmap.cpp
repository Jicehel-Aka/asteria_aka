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
  struct S{int m,x,y;}; S shots[]={ {1,20,10}, {1,16,22}, {5,11,10}, {5,8,36}, {8,9,8}, {8,18,11} };
  for(S s: shots){ WorldScene& w=world_scene(); w.load_map(s.m); w.px=s.x; w.py=s.y; w.notice=false; w.dlg=false;
    host_inject(-1); mgr.update(); mgr.render(); gb::frame_end(); }
  return 0;
}
