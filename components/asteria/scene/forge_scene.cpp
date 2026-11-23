// Forge & Alchimie chez Doran : defilement sur les 7 recettes utiles, accents natifs et stock requis.
#include "scene/forge_scene.h"
#include "i18n.h"
#include "platform/gb_port.h"
#include "ui/text.h"
#include "player.h"
#include "audio/jingle.h"
#include "content_i18n.h"
#include "generated/asteria_recipes_gen.h"
#include <cstdio>

namespace asteria {
using namespace data;
static int sel=0; static char msg[64]={0};

ForgeScene& forge_scene(){ static ForgeScene s; return s; }
void ForgeScene::enter(){ sel=0; msg[0]=0; }

static bool can_make(const Recipe& r){
  for(int i=0;i<r.input_n;i++) if(!player().inv.has(r.inputs[i].item,r.inputs[i].qty)) return false;
  return true;
}

static int inv_qty(int item){
  int idx = player().inv.find(item);
  return idx >= 0 ? (int)player().inv.slots[idx].qty : 0;
}

void ForgeScene::update(SceneManager& m){
  uint32_t p=gb::buttons_pressed();
  if(p&gb::BTN_DOWN){ sel=(sel+1)%RECIPE_COUNT; msg[0]=0; audio::sfx_select(); }
  if(p&gb::BTN_UP){   sel=(sel+RECIPE_COUNT-1)%RECIPE_COUNT; msg[0]=0; audio::sfx_select(); }
  if(p&gb::BTN_A){
    if(try_craft(player(), sel, true)){
      audio::sfx_coin();
      const Recipe& r=RECIPES[sel];
      if(r.out_qty>1) snprintf(msg,64,trc("Fabriqué : %dx %s !"), r.out_qty, trc(ITEMS[r.out_item].name));
      else snprintf(msg,64,trc("Fabriqué : %s !"), trc(ITEMS[r.out_item].name));
    } else {
      snprintf(msg,64,"%s",trc("Ingrédients manquants."));
    }
  }
  if(p&gb::BTN_B) m.set(SceneId::WORLD);
}

void ForgeScene::render(){
  gb::clear(gb::rgb(26,18,14));
  ui::text_big(20,8,trc("Forge & Alchimie"),gb::rgb(235,205,135));
  char cnt[16]; snprintf(cnt,sizeof(cnt),"%d/%d",sel+1,RECIPE_COUNT);
  ui::text(276,12,cnt,gb::rgb(175,165,135));

  const int VIS=4;
  int top = (sel >= VIS) ? (sel - VIS + 1) : 0;
  if(top + VIS > RECIPE_COUNT) top = RECIPE_COUNT - VIS;
  if(top < 0) top = 0;

  for(int row=0; row<VIS && (top+row)<RECIPE_COUNT; row++){
    int i=top+row; int y=38+row*36; const Recipe& r=RECIPES[i]; bool ok=can_make(r);
    if(i==sel){ gb::fill_rect(10,y-3,300,32,gb::rgb(58,42,20)); ui::gold(14,y,">",true); }
    char title[64];
    if(r.out_qty>1) snprintf(title,sizeof(title),"%dx %s",(int)r.out_qty,trc(ITEMS[r.out_item].name));
    else snprintf(title,sizeof(title),"%s",trc(ITEMS[r.out_item].name));
    ui::text(28,y,title,ok?gb::rgb(240,230,190):gb::rgb(155,145,125));
    const char* st = trc((r.station==0) ? "[Forge]" : "[Alchimie]");
    ui::text(304-ui::text_w(st), y, st, (r.station==0)?gb::rgb(215,155,95):gb::rgb(140,205,145));

    int x=28;
    for(int k=0;k<r.input_n;k++){
      int it=r.inputs[k].item, need=r.inputs[k].qty, have=inv_qty(it);
      bool hasIt = have >= need;
      char seg[44]; snprintf(seg,sizeof(seg),"%s%s %d/%d", k?" + ":"", trc(ITEMS[it].name), have, need);
      ui::text(x,y+15,seg, hasIt?gb::rgb(160,215,155):gb::rgb(185,115,105));
      x += ui::text_w(seg) + 2;
    }
  }

  gb::fill_rect(10,184,300,26,gb::rgb(18,14,12));
  if(msg[0]) ui::text(16,190,msg,gb::rgb(190,235,175));
  else ui::text(16,190,trc(item_desc(RECIPES[sel].out_item)),gb::rgb(205,195,165));

  ui::text(16,220,trc("A: fabriquer   Haut/Bas: choisir   B: sortir"),gb::rgb(175,165,135));
}
}
