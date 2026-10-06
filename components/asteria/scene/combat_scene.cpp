// Combat tour par tour (Defis & Sortileges) : 3D6 <= CO pour toucher, degats = des d'arme.
#include "scene/combat_scene.h"
#include "i18n.h"
#include "platform/gb_port.h"
#include "ui/text.h"
#include "player.h"
#include "gamestate.h"
#include "scene/world_scene.h"
#include "generated/asteria_monster.h"
#include "audio/jingle.h"
#include "progress.h"
#include "player.h"
#include <cstdio>
#include <cstring>
namespace asteria {
static int g_enc=0; static const char* g_bmp="/sdcard/ASTERIA/battle/FORE_J.BMP";
static SceneId g_return=SceneId::WORLD;
void start_combat(int idx,const char* bmp){ g_enc=idx; if(bmp) g_bmp=bmp; }
void set_combat_return(SceneId s){ g_return=s; }
CombatScene& combat_scene(){ static CombatScene s; return s; }
static uint32_t g_rng=1;
static int d6(){ g_rng^=g_rng<<13; g_rng^=g_rng>>17; g_rng^=g_rng<<5; return (int)(g_rng%6)+1; }
static int roll3(){ return d6()+d6()+d6(); }
static int rollN(int n){ int s=0; for(int i=0;i<n;i++) s+=d6(); return s; }
// phases: 0 menu, 1 msg joueur, 2 msg ennemi, 3 fin
void CombatScene::enter(){ phase=0; sel=0; result=0; g_rng=gb::millis()|1u; mhp=(unsigned)mon::MON[g_enc].VIT; msg[0]=0; stunned=false; spellSel=0; }
void CombatScene::update(SceneManager& m){
  uint32_t p=gb::buttons_pressed();
  const mon::Stats& M=mon::MON[g_enc];
  if(phase==0){
    if(p&gb::BTN_DOWN) sel=(sel+1)%4;
    if(p&gb::BTN_UP)   sel=(sel+3)%4;
    if(p&gb::BTN_A){
      if(sel==0){ int r=roll3(); if(r<=player().CO){ lastDmg=rollN(1); if((unsigned)lastDmg>=mhp)mhp=0; else mhp-=lastDmg; snprintf(msg,64,"Touche ! (%d<=CO) -%d",r,lastDmg); audio::sfx_hit(); } else snprintf(msg,64,"Manque ! (%d>CO)",r); phase=1; }
      else if(sel==3){ int r=roll3(); if(r<=player().IN){ snprintf(msg,64,"Tu prends la fuite."); result=3; phase=3; } else { snprintf(msg,64,"Fuite ratee ! (%d>IN)",r); phase=1; } }
      else if(sel==2){ if(player().inv.has(4,1)){ player().inv.remove(4,1); int heal=8; player().BL-=heal; if(player().BL<0)player().BL=0; snprintf(msg,64,"Tu bois une potion. Blessures -%d",heal); phase=1; }
                       else { snprintf(msg,64,"Aucune potion !"); phase=4; } }
      else { spellSel=0; phase=5; }
    }
  } else if(phase==1){ // apres action joueur -> mort du monstre ? sinon tour ennemi
    if(p&gb::BTN_A){
      if(mhp==0){ int xp=M.VIT/2+M.CO; int gold=M.VIT/4+d6(); player().xp+=xp; player().gold+=gold;
        int lvl0=player().level;
        while(player().xp>=player().level*100){ player().xp-=player().level*100; player().level++; player().VIT+=5; }
        if(g_enc==9){ flag_set("boss_mine");
          // Butin de boss : Clef du Puits (ouvre la Route) + Pierre noire gravee (fil rouge de l'Arc).
          int kp=item_index("OBJ_0015"), pn=item_index("OBJ_0014"), ca=item_index("OBJ_0010");
          if(kp>=0) player().inv.add(kp,1); if(pn>=0) player().inv.add(pn,1);
          if(ca>=0 && !player().inv.has(ca,1)) player().inv.add(ca,1);   // casque de mineur (Q1 Doran)
          complete_quest_reward(6);
          snprintf(msg,64,"Chef Gobelin vaincu ! Clef du Puits + Pierre noire"); }
        else snprintf(msg,64,"Victoire ! +%d XP  +%d or",xp,gold);
        audio::play(player().level>lvl0 ? audio::TUNE_LEVELUP : audio::TUNE_VICTORY);
        result=1; phase=3; }
      else if(stunned){ stunned=false; snprintf(msg,64,"%s est engourdi et ne bouge pas.",M.name); phase=2; }
      else { int r=roll3(); if(r<=M.CO){ int dmg=rollN(M.BLd); player().BL+=dmg; snprintf(msg,64,"%s attaque ! -%d",M.name,dmg); audio::sfx_hurt();
               if(player().BL>player().VIT){ result=2; phase=3; } else phase=2; }
             else { snprintf(msg,64,"%s te manque. (%d>CO)",M.name,r); phase=2; } }
    }
  } else if(phase==2){ if(p&gb::BTN_A) phase=0; }
  else if(phase==4){ if(p&gb::BTN_A) phase=0; }
  else if(phase==5){
    if(p&gb::BTN_DOWN) spellSel=(spellSel+1)%3;
    if(p&gb::BTN_UP)   spellSel=(spellSel+2)%3;
    if(p&gb::BTN_B) phase=0;
    if(p&gb::BTN_A){ static const int COST[3]={3,3,3}; int cost=COST[spellSel];
      if(player().mana<cost){ snprintf(msg,64,"Pas assez de pouvoir !"); phase=4; }
      else { player().mana-=cost; int r=roll3()+player().level;
        if(r<=player().MA){
          if(spellSel==0){ player().BL-=8; if(player().BL<0)player().BL=0; snprintf(msg,64,"Soins ! Blessures -8"); }
          else if(spellSel==1){ stunned=true; snprintf(msg,64,"Engourdissement reussi !"); }
          else { int d=6; if((unsigned)d>=mhp)mhp=0; else mhp-=d; snprintf(msg,64,"Repulsion ! -%d",d); }
        } else snprintf(msg,64,"Le sort echoue ! (%d>MA)",r);
        phase=1; } }
  }
  else { if(p&gb::BTN_A){ if(result==2){ m.set(SceneId::GAMEOVER); } else m.set(g_return); } }
}
void CombatScene::render(){
  gb::blit_bmp(g_bmp);
  const mon::Stats& M=mon::MON[g_enc]; const spr::Sprite& S=spr::MONSTER[g_enc];
  gb::draw_image_key(S.px,S.w,S.h,160-S.w/2,152-S.h,spr::KEY);   // pieds au sol
  // ennemi : nom + barre PV
  gb::fill_rect(8,8,150,26,gb::rgb(20,16,10)); gb::fill_rect(8,8,150,2,gb::rgb(200,170,90));
  ui::gold(14,12,M.name,true);
  int bw=140, f=(int)((mhp*bw)/(unsigned)M.VIT); gb::fill_rect(160,14,bw,10,gb::rgb(60,20,20)); gb::fill_rect(160,14,f,10,gb::rgb(190,60,50));
  // joueur : VIT/BL
  gb::fill_rect(158,176,154,56,gb::rgb(20,16,10)); gb::fill_rect(158,176,154,2,gb::rgb(200,170,90));
  ui::gold(164,179,"Caithness",true); char b[40];
  int pv=player().VIT-player().BL; if(pv<0)pv=0; int pbw=140, pf=(player().VIT>0)?(pv*pbw)/player().VIT:0;
  gb::fill_rect(164,194,pbw,8,gb::rgb(45,60,45)); gb::fill_rect(164,194,pf,8,gb::rgb(80,180,90));
  snprintf(b,40,"PV %d/%d",pv,player().VIT); ui::text(164,204,b,gb::rgb(230,220,180));
  snprintf(b,40,"Or %d  Niv %d",player().gold,player().level); ui::text(164,218,b,gb::rgb(220,200,120));
  if(phase==0){
    const char* A[4]={i18n::tr(i18n::CB_ATTACK),i18n::tr(i18n::CB_MAGIC),i18n::tr(i18n::CB_ITEM),i18n::tr(i18n::CB_FLEE)};
    gb::fill_rect(8,176,146,56,gb::rgb(20,16,10)); gb::fill_rect(8,176,146,2,gb::rgb(200,170,90));
    for(int i=0;i<4;i++){ if(i==sel){ gb::fill_rect(12,178+i*13,138,12,gb::rgb(60,45,20)); ui::gold(16,179+i*13,">",true);} ui::text(28,179+i*13,A[i],gb::rgb(235,225,190)); }
  } else if(phase==5){
    const char* SP[3]={i18n::tr(i18n::CB_SOINS),i18n::tr(i18n::CB_STUN),i18n::tr(i18n::CB_REPEL)}; static const int COST[3]={3,3,3};
    gb::fill_rect(8,176,300,56,gb::rgb(20,16,10)); gb::fill_rect(8,176,300,2,gb::rgb(200,170,90));
    for(int i=0;i<3;i++){ int y=180+i*15; if(i==spellSel){ gb::fill_rect(12,y-2,180,13,gb::rgb(60,45,20)); ui::gold(14,y-1,">",true);} 
      ui::text(26,y-1,SP[i],gb::rgb(235,225,190)); char c[10]; snprintf(c,10,"%d PO",COST[i]); ui::text(150,y-1,c,gb::rgb(180,160,220)); }
    char mm[28]; snprintf(mm,28,"%s %d/%d",i18n::tr(i18n::CB_POWER),player().mana,player().manaMax); ui::gold(210,182,mm,true);
    ui::text(196,210,i18n::tr(i18n::CB_CAST_H),gb::rgb(170,160,132));
  } else {
    gb::fill_rect(8,176,304,56,gb::rgb(20,16,10)); gb::fill_rect(8,176,304,2,gb::rgb(200,170,90));
    ui::text(16,190,msg,gb::rgb(236,226,192)); ui::text(230,210,i18n::tr(i18n::CB_NEXT),gb::rgb(170,160,132));
  }
}
}
