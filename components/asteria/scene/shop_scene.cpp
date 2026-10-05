#include "scene/shop_scene.h"
#include "platform/gb_port.h"
#include "ui/text.h"
#include "player.h"
#include <cstdio>
namespace asteria {
struct ShopItem { const char* name; int item; int price; };
static const ShopItem SHOP[] = {
  {"Petite potion de soin",4,12}, {"Torche",2,5}, {"Corde",3,5},
  {"Huile pour lanterne",10,6}, {"Arc court",7,30},
};
static const int NSHOP=5; static int sel=0; static char msg[48]={0};
ShopScene& shop_scene(){ static ShopScene s; return s; }
void ShopScene::enter(){ sel=0; msg[0]=0; }
void ShopScene::update(SceneManager& m){
  uint32_t p=gb::buttons_pressed();
  if(p&gb::BTN_DOWN) sel=(sel+1)%NSHOP;
  if(p&gb::BTN_UP)   sel=(sel+NSHOP-1)%NSHOP;
  if(p&gb::BTN_A){ const ShopItem& it=SHOP[sel];
    if(player().gold>=it.price){ player().gold-=it.price; player().inv.add(it.item,1); snprintf(msg,48,"Achete : %s",it.name); }
    else snprintf(msg,48,"Pas assez d'or !"); }
  if(p&gb::BTN_B) m.set(SceneId::WORLD);
}
void ShopScene::render(){
  gb::clear(gb::rgb(30,24,16));
  ui::text_big(64,10,"Echoppe de Mira",gb::rgb(230,210,140));
  char b[32]; snprintf(b,32,"Ton or : %d",player().gold); ui::gold(20,40,b,true);
  for(int i=0;i<NSHOP;i++){ int y=64+i*22; if(i==sel){ gb::fill_rect(16,y-3,288,18,gb::rgb(60,45,20)); ui::gold(20,y,">",true);} 
    ui::text(36,y,SHOP[i].name,gb::rgb(235,225,190)); char pr[12]; snprintf(pr,12,"%d or",SHOP[i].price); ui::text(252,y,pr,gb::rgb(220,200,120)); }
  if(msg[0]) ui::text(20,188,msg,gb::rgb(190,230,180));
  ui::text(20,216,"A: acheter   B: sortir",gb::rgb(180,170,140));
}
}
