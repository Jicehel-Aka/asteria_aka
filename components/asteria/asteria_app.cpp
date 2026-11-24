// ASTERIA (Gamebuino AKA) - Boucle de jeu principale : chaque frame -> sons d'interface
// -> mgr.update() -> horloge / musique / jingles -> mgr.render() -> frame_end().
#include "asteria_app.h"
#include "scene/scene.h"
#include "scene/world_scene.h"
#include "platform/gb_port.h"
#include "audio/jingle.h"
#include "audio/music.h"
#include "daytime.h"
namespace asteria {

// Sons d'interface generiques, deduits des touches pressees AVANT la mise a jour de
// la scene. Les menus "purs" font tic sur les fleches, valident sur A et reculent sur
// B ; dans le monde, les fleches deplacent le heros (silence) sauf menu pause ouvert.
// Les evenements precis (achat, coffre, porte, erreur...) sont joues par les scenes.
static void ui_sounds(SceneId sc){
  uint32_t p = gb::buttons_pressed();
  if(!p) return;
  const uint32_t ARROWS = gb::BTN_UP | gb::BTN_DOWN | gb::BTN_LEFT | gb::BTN_RIGHT;
  switch(sc){
    case SceneId::TITLE: case SceneId::CREATE: case SceneId::RULES: case SceneId::SHOP:
    case SceneId::INN:   case SceneId::FORGE:  case SceneId::DICE:  case SceneId::COMBAT:
      if(p & ARROWS) audio::sfx_select();
      else if(p & gb::BTN_A) audio::sfx_confirm();
      else if(p & gb::BTN_B) audio::sfx_back();
      break;
    case SceneId::WORLD: {
      const WorldScene& w = world_scene();
      if(w.confirmQuit){                                   // menu pause ouvert
        if(p & ARROWS) audio::sfx_select();
        else if(p & gb::BTN_A) audio::sfx_confirm();
        else if(p & (gb::BTN_B | gb::BTN_MENU)) audio::sfx_back();
      } else if(w.dlg || w.notice){
        if(p & gb::BTN_A) audio::sfx_page();               // ligne de dialogue / message suivant
      } else if(p & (gb::BTN_MENU | gb::BTN_B)) audio::sfx_open();
    } break;
    default: break;
  }
}

void run(){
  audio::init();
  SceneManager mgr; mgr.set(SceneId::TITLE);
  while(gb::running()){
    gb::frame_begin();
    ui_sounds(mgr.current());
    mgr.update();
    SceneId sc = mgr.current();
    if(sc==SceneId::WORLD || sc==SceneId::DUNGEON) daytime::tick(gb::millis());   // le temps ne passe qu'en exploration
    music::update(sc);          // theme selon scene / lieu / heure (recharge seulement au changement)
    audio::update();            // jingles + effets de plusieurs notes
    if(sc==SceneId::QUIT){ music::stop(); break; }
    mgr.render();
    gb::frame_end();
  }
}
}
