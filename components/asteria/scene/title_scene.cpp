#include "scene/title_scene.h"
#include "i18n.h"
#include "scene/editor_scene.h"
#include <cstdio>
#include "ui/text.h"
#include "platform/gb_port.h"
namespace asteria {
static const int ITEM_IDS[6]={i18n::MENU_NEW,i18n::MENU_CONT,i18n::MENU_RULES,i18n::MENU_OPT,i18n::MENU_EDITOR,i18n::MENU_QUIT};
static bool enabled(int i,bool has_save){ return !(i==1 && !has_save); }
TitleScene& title_scene(){ static TitleScene s; return s; }
void TitleScene::enter(){ has_save=gb::file_exists("/sdcard/ASTERIA/SAVE.DAT"); if(sel==1&&!has_save) sel=0; }
void TitleScene::update(SceneManager& m){
  uint32_t p=gb::buttons_pressed();
  if(p&gb::BTN_DOWN){ do{ sel=(sel+1)%6; }while(!enabled(sel,has_save)); }
  if(p&gb::BTN_UP){   do{ sel=(sel+5)%6; }while(!enabled(sel,has_save)); }
  if(p&gb::BTN_A){
    switch(sel){
      case 0: m.set(SceneId::CREATE); break;
      case 1: gb::log("TITLE: Continuer -> WORLD"); m.set(SceneId::WORLD); break;
      case 2: gb::log("TITLE: Lire les regles -> RULES"); m.set(SceneId::RULES); break;
      case 3: i18n::next_lang(); break;
      case 4: editor_set_map(0); m.set(SceneId::EDITOR); break;
      case 5: m.set(SceneId::QUIT); break;
    }
  }
  if(p&(gb::BTN_RUN|gb::BTN_MENU)){ /* combo retour loader géré par la plateforme */ }
}
void TitleScene::render(){
  if(!gb::blit_bmp("/sdcard/ASTERIA/TITLE.BMP")) gb::clear(gb::rgb(20,16,32));
  int y=100;
  for(int i=0;i<6;++i){
    if(i==1 && !has_save)      ui::text(28,y,i18n::tr(ITEM_IDS[i]),gb::rgb(120,104,100));   // grisé
    else if(i==sel){ ui::gold(16,y,">",true); ui::gold(28,y,i18n::tr(ITEM_IDS[i]),true); } // sélection
    else                       ui::gold(28,y,i18n::tr(ITEM_IDS[i]),false);                 // or atténué
    y+=17;
  }
  char lb[32]; snprintf(lb,32,"%s: %s",i18n::tr(i18n::OPT_LANG),i18n::tr(i18n::LANG_NAME)); ui::gold(28,y+6,lb,false);
}
}
