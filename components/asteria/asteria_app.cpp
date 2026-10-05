#include "asteria_app.h"
#include "scene/scene.h"
#include "platform/gb_port.h"
namespace asteria {
void run(){
  SceneManager mgr; mgr.set(SceneId::TITLE);
  while(gb::running()){
    gb::frame_begin();
    mgr.update();
    if(mgr.current()==SceneId::QUIT) break;
    mgr.render();
    gb::frame_end();
  }
}
}
