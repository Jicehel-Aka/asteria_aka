// Ecran de mort : fondu rouge sombre, titre, epitaphe, stats finales, puis retour au titre.
#include "scene/gameover_scene.h"
#include "scene/world_scene.h"   // new_game_reset()
#include "platform/gb_port.h"
#include "ui/text.h"
#include "player.h"
#include "i18n.h"
#include "audio/jingle.h"
#include <cstdio>
#include <cstring>
namespace asteria {
GameOverScene& gameover_scene(){ static GameOverScene s; return s; }

void GameOverScene::enter(){ t0=gb::millis(); audio::play(audio::TUNE_DEATH); }

void GameOverScene::update(SceneManager& m){
  uint32_t p=gb::buttons_pressed();
  // petite temporisation pour eviter un skip accidentel (l'appui du dernier combat)
  if((gb::millis()-t0)>600 && (p&(gb::BTN_A|gb::BTN_B|gb::BTN_MENU))){ new_game_reset(); m.set(SceneId::TITLE); }
}

void GameOverScene::render(){
  // fondu : s'assombrit en rouge pendant ~0.8 s
  uint32_t e=gb::millis()-t0; int k=(e<800)?(int)(e*100/800):100;
  gb::clear(gb::rgb(10*k/100, 3*k/100, 5*k/100));
  for(int i=0;i<6;i++) gb::fill_rect(40,58+i,240,1,gb::rgb(60,10,12));   // bandeau sombre derriere le titre
  const char* title=i18n::tr(i18n::GO_TITLE);
  int tw=ui::text_w(title)*18/14; ui::text_big(160-tw/2,60,title,gb::rgb(190,44,40));
  const char* sub=i18n::tr(i18n::GO_SUB);
  ui::text(160-ui::text_w(sub)/2,98,sub,gb::rgb(200,180,164));
  char b[48];
  snprintf(b,48,"%s %d",i18n::tr(i18n::GO_LEVEL),player().level);
  ui::gold(160-ui::text_w(b)/2,132,b,true);
  snprintf(b,48,"%s %d",i18n::tr(i18n::GO_GOLD),player().gold);
  ui::text(160-ui::text_w(b)/2,152,b,gb::rgb(220,200,120));
  if((gb::millis()-t0)>600 && ((gb::millis()/500)&1)){
    const char* c=i18n::tr(i18n::GO_CONT);
    ui::text(160-ui::text_w(c)/2,206,c,gb::rgb(170,160,132));
  }
}
}
