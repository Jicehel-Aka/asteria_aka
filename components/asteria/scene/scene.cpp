#include "scene/scene.h"
#include "scene/title_scene.h"
#include "scene/create_scene.h"
#include "scene/combat_scene.h"
#include "scene/shop_scene.h"
#include "scene/inn_scene.h"
#include "scene/dice_scene.h"
#include "scene/editor_scene.h"
#include "scene/rules_scene.h"
#include "scene/world_scene.h"
#include "scene/dungeon_scene.h"
#include "scene/gameover_scene.h"
#include "platform/gb_port.h"
namespace asteria {
void SceneManager::set(SceneId id){
  cur_=id;
  switch(id){
    case SceneId::TITLE: s_=&title_scene(); break;
    case SceneId::CREATE: s_=&create_scene(); break;
    case SceneId::RULES: s_=&rules_scene(); break;
    case SceneId::WORLD: s_=&world_scene(); break;
    case SceneId::COMBAT: s_=&combat_scene(); break;
    case SceneId::SHOP: s_=&shop_scene(); break;
    case SceneId::INN: s_=&inn_scene(); break;
    case SceneId::DICE: s_=&dice_scene(); break;
    case SceneId::EDITOR: s_=&editor_scene(); break;
    case SceneId::DUNGEON: s_=&dungeon_scene(); break;
    case SceneId::GAMEOVER: s_=&gameover_scene(); break;
    case SceneId::QUIT:  s_=nullptr; gb::return_to_loader(); return;
  }
  if(s_) s_->enter();
}
}
