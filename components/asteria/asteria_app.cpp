// ASTERIA (Gamebuino AKA) - Boucle de jeu principale : chaque frame -> mgr.update() -> audio::update() -> mgr.render() -> frame_end().
#include "asteria_app.h"
#include "scene/scene.h"
#include "platform/gb_port.h"
#include "audio/jingle.h"
namespace asteria {
void run(){
  audio::init();
  SceneManager mgr; mgr.set(SceneId::TITLE);
  while(gb::running()){
    gb::frame_begin();
    mgr.update();
    audio::update();            // avance la mélodie en cours (jingles)
    if(mgr.current()==SceneId::QUIT) break;
    mgr.render();
    gb::frame_end();
  }
}
}
