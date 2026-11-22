// Captures : combat boss mine (CHEF.BMP), ecran creation (CAITH.BMP), nouvelle foret.
#include "scene/scene.h"
#include "scene/world_scene.h"
#include "platform/gb_port.h"
#include "player.h"
#include "gamestate.h"
#include "audio/jingle.h"
using namespace asteria;
void host_set_budget(int n); void host_inject(int code);
namespace asteria { void start_combat(int,const char*); }
static void shot(SceneManager& m){ host_inject(-1); m.update(); m.render(); gb::frame_end(); }
int main(){
  audio::init(); host_set_budget(1000000000); new_game_reset();
  SceneManager mgr;
  // 1) combat contre le Chef Gobelin (illustration plein ecran)
  player().VIT=40; start_combat(9,"/sdcard/ASTERIA/battle/CHEF.BMP"); mgr.set(SceneId::COMBAT); shot(mgr);
  // 2) ecran de creation de personnage (fond Caithness)
  mgr.set(SceneId::CREATE); shot(mgr);
  // 3) foret : une clairiere
  { WorldScene& w=world_scene(); w.load_map(1); w.px=25; w.py=8; } mgr.set(SceneId::WORLD); shot(mgr);
  { WorldScene& w=world_scene(); w.px=12; w.py=29; } shot(mgr);
  return 0;
}
