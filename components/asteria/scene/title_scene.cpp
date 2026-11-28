// ASTERIA (Gamebuino AKA) - Ecran-titre : Nouvelle partie, Continuer (3 emplacements), langue.
#include "scene/title_scene.h"
#include "save.h"
#include "i18n.h"
#include "content_i18n.h"
#include "scene/editor_scene.h"
#include <cstdio>
#include "ui/text.h"
#include "platform/gb_port.h"

namespace asteria {
static const int ITEM_IDS[6]={i18n::MENU_NEW,i18n::MENU_CONT,i18n::MENU_RULES,i18n::MENU_OPT,i18n::MENU_EDITOR,i18n::MENU_QUIT};
static bool enabled(int i,bool has_save){ return !(i==1 && !has_save); }
TitleScene& title_scene(){ static TitleScene s; return s; }

void TitleScene::enter(){
  slot = active_save_slot();
  has_save = false;
  int first_valid = -1;
  for(int s=1; s<=3; ++s){
    save_slot_summary(s, sums[s-1]);
    if(sums[s-1].exists){ has_save = true; if(first_valid < 0) first_valid = s; }
  }
  if(first_valid > 0 && !sums[slot-1].exists){
    slot = first_valid;
    set_active_save_slot(slot);
  }
  if(sel==1 && !has_save) sel=0;
}

void TitleScene::update(SceneManager& m){
  uint32_t p=gb::buttons_pressed();
  if(p&gb::BTN_DOWN){ do{ sel=(sel+1)%6; }while(!enabled(sel,has_save)); }
  if(p&gb::BTN_UP){   do{ sel=(sel+5)%6; }while(!enabled(sel,has_save)); }
  if(sel==1 && has_save){
    if(p&gb::BTN_RIGHT){ slot = (slot % 3) + 1; set_active_save_slot(slot); }
    if(p&gb::BTN_LEFT){  slot = ((slot + 1) % 3) + 1; set_active_save_slot(slot); }
  }
  if(p&gb::BTN_A){
    switch(sel){
      case 0: m.set(SceneId::CREATE); break;
      case 1:
        if(game_load_slot(slot)){ gb::log("TITLE: sauvegarde chargée -> WORLD"); m.set(SceneId::WORLD); }
        else gb::log("TITLE: échec chargement");
        break;
      case 2: gb::log("TITLE: Lire les règles -> RULES"); m.set(SceneId::RULES); break;
      case 3: i18n::next_lang(); break;
      case 4: editor_set_map(0); m.set(SceneId::EDITOR); break;
      case 5: m.set(SceneId::QUIT); break;
    }
  }
  if(p&(gb::BTN_RUN|gb::BTN_MENU)){ /* combo retour loader gere par la plateforme */ }
}

void TitleScene::render(){
  if(!gb::blit_bmp("/sdcard/ASTERIA/TITLE.BMP")) gb::clear(gb::rgb(20,16,32));
  int y=98;
  for(int i=0;i<6;++i){
    if(i==1 && !has_save)
      ui::text(28,y,i18n::tr(ITEM_IDS[i]),gb::rgb(120,104,100));
    else if(i==sel){ ui::gold(16,y,">",true); ui::gold(28,y,i18n::tr(ITEM_IDS[i]),true); }   // sélection
    else ui::gold(28,y,i18n::tr(ITEM_IDS[i]),false);   // or atténué

    if(i==1 && has_save){
      char sl[40];
      const SaveSummary& sm = sums[slot-1];
      if(sm.exists) snprintf(sl,sizeof(sl),trc("<S%d: %s Nv.%d>"),slot,sm.name,sm.level);
      else snprintf(sl,sizeof(sl),trc("<S%d: Vide>"),slot);
      ui::text(122,y,sl,(i==sel)?gb::rgb(240,220,140):gb::rgb(150,140,120));
    }
    y+=17;
  }
  char lb[32]; snprintf(lb,32,"%s: %s",i18n::tr(i18n::OPT_LANG),i18n::tr(i18n::LANG_NAME));
  ui::gold(28,y+4,lb,false);
}
}
