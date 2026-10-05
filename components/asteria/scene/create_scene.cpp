#include "scene/create_scene.h"
#include "ui/text.h"
#include "platform/gb_port.h"
#include "player.h"
#include "scene/world_scene.h"
#include <cstdio>
namespace asteria {
static const int BASE=8, MINV=8, MAXV=30, POOL0=18;
CreateScene& create_scene(){ static CreateScene s; return s; }
void CreateScene::enter(){ new_game_reset(); step=0; selChoice=0; cur=0; st[0]=st[1]=st[2]=st[3]=BASE; pool=POOL0; }
static void buildCustom(const int* st){ Character c=new_caithness(); c.IN=st[0];c.CO=st[1];c.MA=st[2];c.AU=st[3]; player()=c; }
void CreateScene::update(SceneManager& m){
  uint32_t p=gb::buttons_pressed();
  if(step==0){
    if(p&gb::BTN_DOWN) selChoice=1;
    if(p&gb::BTN_UP)   selChoice=0;
    if(p&gb::BTN_B)    m.set(SceneId::TITLE);
    if(p&gb::BTN_A){ if(selChoice==0){ player()=new_caithness(); step=2; } else { step=1; cur=0; } }
  } else if(step==1){
    if(p&gb::BTN_DOWN) cur=(cur+1)%6;
    if(p&gb::BTN_UP)   cur=(cur+5)%6;
    if(p&gb::BTN_A){ if(cur<4){ if(pool>0&&st[cur]<MAXV){st[cur]++;pool--;} } else if(cur==4){ buildCustom(st); step=2; } else { step=0; } }
    if(p&gb::BTN_B){ if(cur<4 && st[cur]>MINV){ st[cur]--; pool++; } }
  } else {
    if(p&gb::BTN_A) m.set(SceneId::WORLD);
    if(p&gb::BTN_B) step=(selChoice==1)?1:0;
  }
}
void CreateScene::render(){
  gb::clear(gb::rgb(26,20,14));
  if(step==0){
    ui::text_big(64,14,"NOUVEAU PERSONNAGE",gb::rgb(230,210,140));
    const char* o[2]={"Personnage pretire (Caithness)","Creer un personnage"};
    for(int i=0;i<2;i++){ int y=72+i*20;
      if(i==selChoice){ gb::fill_rect(18,y-3,286,17,gb::rgb(60,45,20)); ui::gold(22,y,">",true); }
      ui::text(38,y,o[i],gb::rgb(236,226,192)); }
    ui::text(18,214,"A: valider    B: retour",gb::rgb(180,170,140));
  } else if(step==1){
    ui::text_big(56,10,"REPARTIS TES POINTS",gb::rgb(230,210,140));
    char buf[40]; snprintf(buf,sizeof(buf),"Points restants : %d",pool); ui::text(92,34,buf,gb::rgb(224,214,182));
    const char* rows[6]={"IN","CO","MA","AU","Valider","Retour"};
    for(int i=0;i<6;i++){ int y=58+i*20;
      if(i==cur){ gb::fill_rect(40,y-3,244,17,gb::rgb(60,45,20)); ui::gold(44,y,">",true); }
      if(i<4){ char l[24]; snprintf(l,sizeof(l),"%s : %d",rows[i],st[i]); ui::text(62,y,l,gb::rgb(236,226,192)); }
      else ui::text(62,y,rows[i],gb::rgb(236,226,192)); }
    ui::text(8,216,"A: +1 / valider   B: -1 / retour",gb::rgb(180,170,140));
  } else {
    Character& c=player();
    ui::text_big(96,10,"FEUILLE",gb::rgb(230,210,140));
    char b[40];
    snprintf(b,sizeof(b),"IN %d    CO %d",c.IN,c.CO); ui::text(34,44,b,gb::rgb(236,226,192));
    snprintf(b,sizeof(b),"MA %d    AU %d",c.MA,c.AU); ui::text(34,64,b,gb::rgb(236,226,192));
    snprintf(b,sizeof(b),"VIT %d   BL %d",c.VIT,c.BL); ui::text(34,84,b,gb::rgb(236,226,192));
    snprintf(b,sizeof(b),"Pouvoir %d",c.PO);          ui::text(34,104,b,gb::rgb(236,226,192));
    ui::text(34,132,"Arme : Dague (1D)",gb::rgb(220,210,180));
    ui::text(34,152,"Sorts : Attirance, Engourdissement,",gb::rgb(210,200,170));
    ui::text(34,166,"Dissipation, Repulsion, Soins",gb::rgb(210,200,170));
    ui::gold(44,208,"A: commencer l'aventure",true);
  }
}
}
