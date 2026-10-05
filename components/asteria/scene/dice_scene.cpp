// Table de dés : mise, 3D6 joueur contre 3D6 la maison, plus haut gagne.
#include "scene/dice_scene.h"
#include "i18n.h"
#include "platform/gb_port.h"
#include "ui/text.h"
#include "player.h"
#include <cstdio>
namespace asteria {
static const int BETS[3]={5,10,20};
static uint32_t rng=1;
static int d6(){ rng^=rng<<13; rng^=rng>>17; rng^=rng<<5; return (int)(rng%6)+1; }
DiceScene& dice_scene(){ static DiceScene s; return s; }
void DiceScene::enter(){ betSel=0; phase=0; pr=hr=0; msg[0]=0; rng=gb::millis()|1u; }
void DiceScene::update(SceneManager& m){
  uint32_t p=gb::buttons_pressed();
  if(phase==0){
    if(p&gb::BTN_DOWN) betSel=(betSel+1)%3;
    if(p&gb::BTN_UP)   betSel=(betSel+2)%3;
    if(p&gb::BTN_B) m.set(SceneId::WORLD);
    if(p&gb::BTN_A){ int bet=BETS[betSel];
      if(player().gold<bet){ snprintf(msg,64,"Pas assez d'or pour cette mise."); phase=2; }
      else { pr=d6()+d6()+d6(); hr=d6()+d6()+d6();
        if(pr>hr){ player().gold+=bet; snprintf(msg,64,"Tu gagnes ! %d contre %d  (+%d or)",pr,hr,bet); }
        else if(pr<hr){ player().gold-=bet; snprintf(msg,64,"Perdu... %d contre %d  (-%d or)",pr,hr,bet); }
        else snprintf(msg,64,"Egalite ! %d partout. Mise rendue.",pr);
        phase=1; } }
  } else { if(p&gb::BTN_A) phase=0; if(p&gb::BTN_B) m.set(SceneId::WORLD); }
}
void DiceScene::render(){
  gb::clear(gb::rgb(26,22,30));
  ui::text_big(40,10,i18n::tr(i18n::DI_TITLE),gb::rgb(230,210,140));
  char b[32]; snprintf(b,32,"Or : %d",player().gold); ui::gold(20,40,b,true);
  if(phase==0){
    ui::text(20,70,i18n::tr(i18n::DI_BET),gb::rgb(220,210,180));
    for(int i=0;i<3;i++){ int y=92+i*20; if(i==betSel){ gb::fill_rect(16,y-3,150,17,gb::rgb(60,45,20)); ui::gold(20,y,">",true);} char l[16]; snprintf(l,16,"%d or",BETS[i]); ui::text(36,y,l,gb::rgb(235,225,190)); }
    ui::text(20,210,i18n::tr(i18n::DI_HINT),gb::rgb(180,170,140));
  } else {
    char t[20]; snprintf(t,20,"%s %d",i18n::tr(i18n::DI_YOU),pr); ui::text_big(60,90,t,gb::rgb(200,225,200));
    snprintf(t,20,"%s %d",i18n::tr(i18n::DI_HOUSE),hr); ui::text_big(60,120,t,gb::rgb(225,200,200));
    gb::fill_rect(12,150,296,30,gb::rgb(20,16,10)); ui::text(20,159,msg,gb::rgb(235,225,190));
    ui::text(20,210,i18n::tr(i18n::DI_AGAIN),gb::rgb(180,170,140));
  }
}
}
