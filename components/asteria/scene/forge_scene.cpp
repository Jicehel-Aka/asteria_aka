// Forge & Alchimie chez Doran : fabrique les recettes (BUG-03 : try_craft enfin relie a un ecran).
#include "scene/forge_scene.h"
#include "i18n.h"
#include "platform/gb_port.h"
#include "ui/text.h"
#include "player.h"
#include "content_i18n.h"
#include "generated/asteria_recipes_gen.h"
#include <cstdio>
namespace asteria {
using namespace data;
static int sel=0; static char msg[48]={0};
// Les noms d'objets sont en UTF-8 (accents) ; le reste du jeu est en ASCII (police CP1252).
// On replie les accents Latin-1 vers l'ASCII pour un affichage propre (pas de mojibake "Ã©").
static const char* fold(const char* s){
  static char buf[64]; int o=0;
  for(const unsigned char* p=(const unsigned char*)s; *p && o<63; ){
    unsigned char c=*p;
    if(c<0x80){ buf[o++]=(char)c; p++; }
    else if(c==0xC3 && p[1]){ unsigned char d=p[1]; char r='?';
      if(d>=0xA0&&d<=0xA5)r='a'; else if(d==0xA7)r='c'; else if(d>=0xA8&&d<=0xAB)r='e';
      else if(d>=0xAC&&d<=0xAF)r='i'; else if(d>=0xB2&&d<=0xB6)r='o'; else if(d>=0xB9&&d<=0xBC)r='u'; else if(d==0xB1)r='n';
      else if(d>=0x80&&d<=0x85)r='A'; else if(d==0x87)r='C'; else if(d>=0x88&&d<=0x8B)r='E';
      else if(d>=0x8C&&d<=0x8F)r='I'; else if(d>=0x92&&d<=0x96)r='O'; else if(d>=0x99&&d<=0x9C)r='U'; else if(d==0x91)r='N';
      buf[o++]=r; p+=2; }
    else if(c>=0xC2&&c<=0xDF && p[1]) p+=2;
    else p++;
  }
  buf[o]=0; return buf;
}
ForgeScene& forge_scene(){ static ForgeScene s; return s; }
void ForgeScene::enter(){ sel=0; msg[0]=0; }
static bool can_make(const Recipe& r){ for(int i=0;i<r.input_n;i++) if(!player().inv.has(r.inputs[i].item,r.inputs[i].qty)) return false; return true; }
void ForgeScene::update(SceneManager& m){
  uint32_t p=gb::buttons_pressed();
  if(p&gb::BTN_DOWN) sel=(sel+1)%RECIPE_COUNT;
  if(p&gb::BTN_UP)   sel=(sel+RECIPE_COUNT-1)%RECIPE_COUNT;
  if(p&gb::BTN_A){ if(try_craft(player(), sel, true)) snprintf(msg,48,"Fabrique : %s", fold(ITEMS[RECIPES[sel].out_item].name));
                   else snprintf(msg,48,"Ingredients manquants."); }
  if(p&gb::BTN_B) m.set(SceneId::WORLD);
}
void ForgeScene::render(){
  gb::clear(gb::rgb(26,18,14));
  ui::text_big(40,8,"Forge & Alchimie",gb::rgb(230,200,140));
  for(int i=0;i<RECIPE_COUNT;i++){ int y=48+i*30; const Recipe& r=RECIPES[i]; bool ok=can_make(r);
    if(i==sel){ gb::fill_rect(14,y-3,292,26,gb::rgb(60,45,20)); ui::gold(18,y,">",true); }
    ui::text(34,y, fold(trc(ITEMS[r.out_item].name)), ok?gb::rgb(235,225,190):gb::rgb(150,140,120));
    char ing[64]={0}; int o=0;
    for(int k=0;k<r.input_n && o<(int)sizeof(ing)-20;k++) o+=snprintf(ing+o,sizeof(ing)-o,"%s%dx %s", k?" + ":"", r.inputs[k].qty, fold(trc(ITEMS[r.inputs[k].item].name)));
    ui::text(34,y+13, ing, ok?gb::rgb(175,165,135):gb::rgb(120,110,95));
  }
  if(msg[0]) ui::text(20,196,msg,gb::rgb(190,230,180));
  ui::text(20,220,"A: fabriquer   B: sortir",gb::rgb(180,170,140));
}
}
