// ASTERIA (Gamebuino AKA) - Auberge : repos (restaure les PV) et rumeurs localisees.
#include "scene/inn_scene.h"
#include "i18n.h"
#include "platform/gb_port.h"
#include "ui/text.h"
#include "content_i18n.h"
#include "player.h"
#include <cstdio>
#include "daytime.h"
#include "audio/jingle.h"
namespace asteria {
static const int REST_COST=5;
static const char* RUMORS[]={
  "On dit que la Vieille Mine cache plus qu'un gobelin.",
  "Le maire ne dort plus depuis les disparitions.",
  "Un marchand jure avoir vu des lueurs au Temple Oublie.",
  "La foret murmure, parait-il, aux voyageurs solitaires.",
  "Selene connait des secrets qu'elle ne dit a personne.",
};
static int g_rum=0;
InnScene& inn_scene(){ static InnScene s; return s; }
void InnScene::enter(){ sel=0; msg[0]=0; }
void InnScene::update(SceneManager& m){
  uint32_t p=gb::buttons_pressed();
  if(p&gb::BTN_DOWN) sel=(sel+1)%3;
  if(p&gb::BTN_UP)   sel=(sel+2)%3;
  if(p&gb::BTN_B) m.set(SceneId::WORLD);
  if(p&gb::BTN_A){
    if(sel==0){ if(player().gold>=REST_COST){ player().gold-=REST_COST; player().BL=0; player().mana=player().manaMax; daytime::set_minutes(8*60); audio::sfx_save(); snprintf(msg,72,"%s",i18n::tr(i18n::IN_RESTED)); }   // une nuit de sommeil : reveil a 8h
      else { snprintf(msg,72,"Pas assez d'or (%d requis).",REST_COST); audio::sfx_error(); } }
    else if(sel==1){ snprintf(msg,72,"\"%s\"",trc(RUMORS[g_rum])); g_rum=(g_rum+1)%5; }
    else m.set(SceneId::WORLD);
  }
}
void InnScene::render(){
  gb::clear(gb::rgb(34,26,18));
  ui::text_big(56,10,i18n::tr(i18n::IN_TITLE),gb::rgb(230,210,140));
  char b[32]; snprintf(b,32,"Or : %d",player().gold); ui::gold(20,40,b,true);
  const char* O[3]={i18n::tr(i18n::IN_REST),i18n::tr(i18n::IN_RUMORS),i18n::tr(i18n::IN_EXIT)};
  for(int i=0;i<3;i++){ int y=70+i*22; if(i==sel){ gb::fill_rect(16,y-3,288,18,gb::rgb(60,45,20)); ui::gold(20,y,">",true);} ui::text(36,y,O[i],gb::rgb(235,225,190)); }
  if(msg[0]){ gb::fill_rect(12,150,296,40,gb::rgb(20,16,10)); ui::text(20,160,msg,gb::rgb(200,225,200)); }
  ui::text(20,216,i18n::tr(i18n::IN_HINT),gb::rgb(180,170,140));
}
}
