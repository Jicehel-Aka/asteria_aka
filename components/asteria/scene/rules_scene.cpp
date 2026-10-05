#include "scene/rules_scene.h"
#include "ui/text.h"
#include "platform/gb_port.h"
#include "generated/asteria_rules_gen.h"
namespace asteria {
using namespace data;
// langue courante : FR pour l'instant (à brancher sur le système i18n)
static const RulesPage* PAGES=RULES_FR; static const int NP=RULES_FR_COUNT;
RulesScene& rules_scene(){ static RulesScene s; return s; }
void RulesScene::enter(){ page=0; }
void RulesScene::update(SceneManager& m){
  uint32_t p=gb::buttons_pressed();
  if(p&(gb::BTN_A|gb::BTN_RIGHT)){ if(page<NP-1){ page++; } else { gb::log("RULES: fin -> TITLE"); m.set(SceneId::TITLE);} }
  if(p&gb::BTN_LEFT){ if(page>0) page--; }
  if(p&gb::BTN_B){ gb::log("RULES: retour -> TITLE"); m.set(SceneId::TITLE); }
}
void RulesScene::render(){
  gb::clear(gb::rgb(18,14,26));
  const RulesPage& pg=PAGES[page];
  ui::gold(12,10,pg.heading,true);
  int y=40; for(int i=0;i<pg.line_n;++i){ ui::text(14,y,pg.lines[i],gb::rgb(232,226,208)); y+=16; }
  char foot[24]; int a=page+1;
  foot[0]='('; foot[1]=(char)('0'+a); foot[2]='/'; foot[3]=(char)('0'+NP); foot[4]=')'; foot[5]=0;
  ui::text(270,222,foot,gb::rgb(180,170,150));
}
}
