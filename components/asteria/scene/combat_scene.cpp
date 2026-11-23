// Combat tour par tour (Defis & Sortileges) : 3D6 <= CO pour toucher, degats = des d'arme.
#include "scene/combat_scene.h"
#include "i18n.h"
#include "content_i18n.h"
#include "platform/gb_port.h"
#include "ui/text.h"
#include "player.h"
#include "gamestate.h"
#include "scene/world_scene.h"
#include "generated/asteria_monster.h"
#include "audio/jingle.h"
#include "progress.h"
#include "generated/asteria_spells_gen.h"
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
// ---------------------------------------------------------------------------
// Armes (BUG-02) : l'Item genere n'a pas de champ degats -> table locale.
// Equipement automatique : on se bat toujours avec la MEILLEURE arme possedee
// (pas d'ecran d'equipement sur console 8 boutons). Les arcs tirent avec un
// bonus au toucher (attaque a distance) et +1 degat si on a des fleches renforcees.
// ---------------------------------------------------------------------------
struct WeaponDef{ const char* id; int bonus; int hit; bool bow; };
static const WeaponDef WEAPONS[]={
  {"OBJ_0037",0,0,false},   // Dague (depart) : 1D
  {"OBJ_0001",2,0,false},   // Epee usee      : 1D+2
  {"OBJ_0008",2,1,true },   // Arc court      : 1D+2, +1 toucher
  {"OBJ_0002",3,0,false},   // Hache          : 1D+3
  {"OBJ_0020",3,2,true },   // Arc de chasseur: 1D+3, +2 toucher
  {"OBJ_0032",4,0,false},   // Epee de fer    : 1D+4
};
static const int WEAPON_N=(int)(sizeof(WEAPONS)/sizeof(WEAPONS[0]));
// Anneau de l'Aube (OBJ_0040) : aide de debut de partie — +1 degat, +2 au toucher.
static int ring_bonus(){ return player().inv.has(item_index("OBJ_0040"),1)?1:0; }
// Meilleure arme possedee (ou l'arme d'origine si aucune autre) ; met a jour player().weapon.
static const WeaponDef& best_weapon(){
  int best=0;
  for(int i=1;i<WEAPON_N;i++){ int it=item_index(WEAPONS[i].id);
    if(it>=0 && (player().inv.has(it,1) || player().weapon==it) && WEAPONS[i].bonus+WEAPONS[i].hit>=WEAPONS[best].bonus+WEAPONS[best].hit) best=i; }
  player().weapon=item_index(WEAPONS[best].id);
  return WEAPONS[best];
}
static int weapon_hit(){ return best_weapon().hit; }
static int weapon_dmg(){ const WeaponDef& w=best_weapon(); int d=rollN(1)+w.bonus+ring_bonus();
  if(w.bow && player().inv.has(item_index("OBJ_0027"),1)) d+=1;   // fleches renforcees
  return d; }

// ---------------------------------------------------------------------------
// Sorts (BUG-02) : les 12 sorts de data::SPELLS. Connus = masque player().spells
// (sorts de base des la creation) + sorts avances debloques par le niveau.
// Cout : 3 pouvoir pour un sort de base, po_cost pour un sort avance.
// Jet : 3D6 <= MA + niveau (le niveau aide). Effets adaptes au combat 1 contre 1.
// ---------------------------------------------------------------------------
// Noms courts (le menu est une grille 2 colonnes de ~140 px) ; traduits par trc().
static const char* const SPELL_SHORT[data::SPELL_COUNT]={
  "Attirance","Engourdissement","Dissipation","Repulsion","Soins","Acceleration",
  "Elementaire","Guerison majeure","Metamorphose","Invisibilite","Mimetisme","Vivacite" };
// Effet resume (ASCII : la police n'a pas les accents UTF-8 des donnees generees).
static const char* const SPELL_FX[data::SPELL_COUNT]={
  "+2 au toucher pendant 3 tours","L'ennemi perd son prochain tour","1D+2 degats, annule ses ruses",
  "2D degats a la cible","Soigne 2D blessures","Tu rejoues avant l'ennemi","Un elementaire frappe 3 tours (2D)",
  "Soigne 4D blessures","Ours : +3 deg, +2 toucher, bouclier 3D","L'ennemi rate 2 attaques, fuite assuree",
  "-4 CO a l'ennemi pendant 3 tours","+3 toucher, +1 degat pendant 3 tours" };
bool spell_known(int i){
  if(i<0||i>=data::SPELL_COUNT) return false;
  if(player().spells&(1u<<i)) return true;
  return data::SPELLS[i].tier==data::ST_ADVANCED && player().level>=data::SPELLS[i].level;
}
int spell_cost(int i){ return data::SPELLS[i].tier==data::ST_BASE ? 3 : data::SPELLS[i].po_cost; }
const char* spell_short(int i){ return (i>=0&&i<data::SPELL_COUNT)?SPELL_SHORT[i]:""; }
// Liste des sorts connus (indices data::SPELLS), renvoie leur nombre.
static int known_list(int* out){ int n=0; for(int i=0;i<data::SPELL_COUNT;i++) if(spell_known(i)) out[n++]=i; return n; }

static void hurt_monster(unsigned& mhp,int d){ if(d<0)d=0; if((unsigned)d>=mhp) mhp=0; else mhp-=(unsigned)d; }
static void heal_player(int d){ player().BL-=d; if(player().BL<0) player().BL=0; }

// Lance le sort s (deja paye et reussi) : applique l'effet et ecrit le message.
void CombatScene::cast(int s){
  switch(s){
    case 0:  hitBuff=3;  hitBonus=2; snprintf(msg,64,"Attirance : +2 au toucher (3 tours)"); break;
    case 1:  stunned=true; snprintf(msg,64,"Engourdissement reussi !"); break;
    case 2:  { int d=rollN(1)+2; hurt_monster(mhp,d); enemyDebuff=0; snprintf(msg,64,"Dissipation ! La magie ennemie se dechire -%d",d); } break;
    case 3:  { int d=rollN(2); hurt_monster(mhp,d); snprintf(msg,64,"Repulsion ! -%d",d); } break;
    case 4:  { int h=rollN(2); heal_player(h); snprintf(msg,64,"Soins ! Blessures -%d",h); } break;
    case 5:  haste=true; snprintf(msg,64,"Acceleration : tu rejoues avant l'ennemi !"); break;
    case 6:  summon=3; snprintf(msg,64,"Un elementaire surgit a tes cotes (3 tours)"); break;
    case 7:  { int h=rollN(4); heal_player(h); snprintf(msg,64,"Guerison majeure ! Blessures -%d",h); } break;
    case 8:  bear=3; shield=rollN(3); snprintf(msg,64,"Metamorphose en ours ! (+3 deg, bouclier %d)",shield); break;
    case 9:  invis=2; snprintf(msg,64,"Invisibilite : l'ennemi frappe dans le vide"); break;
    case 10: enemyDebuff=3; snprintf(msg,64,"Mimetisme : l'ennemi se trompe (-4 CO, 3 tours)"); break;
    case 11: hitBuff=3; hitBonus=3; dmgBonus=1; snprintf(msg,64,"Vivacite : +3 au toucher, +1 degat (3 tours)"); break;
  }
}

// phases: 0 menu, 1 msg joueur, 2 msg ennemi, 3 fin, 4 msg sans tour ennemi, 5 menu magie
void CombatScene::enter(){ phase=0; sel=0; result=0; g_rng=gb::millis()|1u; mhp=(unsigned)mon::MON[g_enc].VIT; msg[0]=0;
  stunned=false; spellSel=0; hitBuff=0; hitBonus=0; dmgBonus=0; summon=0; bear=0; shield=0; invis=0; enemyDebuff=0; haste=false; summonActed=false; }
void CombatScene::update(SceneManager& m){
  uint32_t p=gb::buttons_pressed();
  const mon::Stats& M=mon::MON[g_enc];
  if(phase==0){
    if(p&gb::BTN_DOWN) sel=(sel+1)%4;
    if(p&gb::BTN_UP)   sel=(sel+3)%4;
    if(p&gb::BTN_A){
      if(sel==0){ int r=roll3(); int toHit=player().CO+2*ring_bonus()+weapon_hit()+(hitBuff>0?hitBonus:0)+(bear>0?2:0);
        if(r<=toHit){ lastDmg=weapon_dmg()+(hitBuff>0?dmgBonus:0)+(bear>0?3:0); hurt_monster(mhp,lastDmg); snprintf(msg,64,"Touche ! (%d<=%d) -%d",r,toHit,lastDmg); audio::sfx_hit(); }
        else snprintf(msg,64,"Manque ! (%d>%d)",r,toHit);
        phase=1; }
      else if(sel==3){ int r=roll3(); if(r<=player().IN || invis>0){ snprintf(msg,64,"Tu prends la fuite."); result=3; phase=3; } else { snprintf(msg,64,"Fuite ratee ! (%d>IN)",r); phase=1; } }
      else if(sel==2){ int big=item_index("OBJ_0033"), pot=item_index("OBJ_0005");
        if(big>=0 && player().inv.has(big,1) && player().BL>8){ player().inv.remove(big,1); heal_player(16); snprintf(msg,64,"Grande potion ! Blessures -16"); phase=1; }
        else if(pot>=0 && player().inv.has(pot,1)){ player().inv.remove(pot,1); heal_player(8); snprintf(msg,64,"Tu bois une potion. Blessures -8"); phase=1; }
        else if(big>=0 && player().inv.has(big,1)){ player().inv.remove(big,1); heal_player(16); snprintf(msg,64,"Grande potion ! Blessures -16"); phase=1; }
        else { snprintf(msg,64,"Aucune potion !"); phase=4; } }
      else { spellSel=0; phase=5; }
    }
  } else if(phase==1){ // apres action joueur -> mort du monstre ? sinon tour ennemi
    if(p&gb::BTN_A){
      if(mhp>0 && summon>0 && !summonActed){ summonActed=true; summon--; int d=rollN(2); hurt_monster(mhp,d); snprintf(msg,64,"L'elementaire frappe ! -%d",d); return; }   // reste en phase 1
      if(mhp==0){ int xp=M.VIT/2+M.CO; int gold=M.VIT/4+d6(); player().xp+=xp; player().gold+=gold;
        int lvl0=player().level;
        while(player().xp>=player().level*100){ player().xp-=player().level*100; player().level++; player().VIT+=5; player().manaMax+=2; }
        if(g_enc==9){ flag_set("boss_mine");
          // Butin de boss : Clef du Puits (ouvre la Route) + Pierre noire gravee (fil rouge de l'Arc).
          int kp=item_index("OBJ_0015"), pn=item_index("OBJ_0014"), ca=item_index("OBJ_0010");
          if(kp>=0) player().inv.add(kp,1); if(pn>=0) player().inv.add(pn,1);
          if(ca>=0 && !player().inv.has(ca,1)) player().inv.add(ca,1);   // casque de mineur (Q1 Doran)
          complete_quest_reward(6);
          snprintf(msg,64,"Chef Gobelin vaincu ! Clef du Puits + Pierre noire"); }
        else if(g_enc==19){ flag_set("boss_gardien"); snprintf(msg,64,"Le Gardien du Seuil tombe. La voie du coeur s'ouvre."); }
        else if(g_enc==24){ flag_set("boss_coeur"); snprintf(msg,64,"Tu as libere cet univers du joug de son tyran."); }
        else if(g_enc==25){ flag_set("boss_corbeau"); complete_quest_reward(11); snprintf(msg,64,"Corbeau Noir tombe ! La route est libre."); }
        else if(player().level>lvl0) snprintf(msg,64,"Victoire ! Niveau %d atteint !",player().level);
        else snprintf(msg,64,"Victoire ! +%d XP  +%d or",xp,gold);
        if(g_enc!=9 && g_enc!=25) audio::play(player().level>lvl0 ? audio::TUNE_LEVELUP : audio::TUNE_VICTORY);  // boss : jingle joue par complete_quest_reward
        result=1; phase=3; }
      else if(haste){ haste=false; snprintf(msg,64,"Tu es plus rapide : a toi de jouer !"); phase=2; }
      else if(stunned){ stunned=false; snprintf(msg,64,"%s est engourdi et ne bouge pas.",M.name); phase=2; }
      else if(invis>0){ invis--; snprintf(msg,64,"%s frappe dans le vide !",M.name); phase=2; }
      else { int co=M.CO-(enemyDebuff>0?4:0); int r=roll3();
        if(r<=co){ int dmg=rollN(M.BLd);
          if(shield>0){ int a=dmg<shield?dmg:shield; shield-=a; dmg-=a; }         // Metamorphose : la peau d'ours encaisse
          player().BL+=dmg; snprintf(msg,64,"%s attaque ! -%d",M.name,dmg); audio::sfx_hurt();
          if(player().BL>=player().VIT){ result=2; phase=3; } else phase=2; }   // BUG-05 : mort des 0 PV (BL>=VIT)
        else { snprintf(msg,64,"%s te manque. (%d>CO)",M.name,r); phase=2; } }
      if(phase==2){ if(hitBuff>0) hitBuff--; if(bear>0) bear--; if(enemyDebuff>0) enemyDebuff--; }   // fin de tour : les effets s'usent
    }
  } else if(phase==2){ if(p&gb::BTN_A){ phase=0; summonActed=false; } }
  else if(phase==4){ if(p&gb::BTN_A){ phase=0; summonActed=false; } }
  else if(phase==5){
    int K[data::SPELL_COUNT]; int n=known_list(K); if(n==0){ phase=0; return; }
    if(spellSel>=n) spellSel=n-1;
    // grille 2 colonnes x 3 lignes (pages de 6) : haut/bas = ligne, gauche/droite = colonne
    if(p&gb::BTN_DOWN)  spellSel=(spellSel+2<n)?spellSel+2:spellSel;
    if(p&gb::BTN_UP)    spellSel=(spellSel>=2)?spellSel-2:spellSel;
    if(p&gb::BTN_RIGHT) spellSel=(spellSel+1<n)?spellSel+1:spellSel;
    if(p&gb::BTN_LEFT)  spellSel=(spellSel>0)?spellSel-1:0;
    if(spellSel>=n) spellSel=n-1;
    if(p&gb::BTN_B) phase=0;
    if(p&gb::BTN_A){ int s=K[spellSel]; int cost=spell_cost(s);
      if(player().mana<cost){ snprintf(msg,64,"Pas assez de pouvoir !"); phase=4; }
      else { player().mana-=cost; int r=roll3();   // roll-under : le niveau AIDE (bonus a MA)
        if(r<=player().MA+player().level) cast(s);
        else snprintf(msg,64,"Le sort echoue ! (%d>MA)",r);
        phase=1; } }
  }
  else { if(p&gb::BTN_A){ if(result==2){ m.set(SceneId::GAMEOVER); } else m.set(g_return); } }
}
void CombatScene::render(){
  gb::blit_bmp(g_bmp);
  const mon::Stats& M=mon::MON[g_enc]; const spr::Sprite& S=spr::MONSTER[g_enc];
  // DELTA-03 : ancrage au sol sur la derniere rangee VISIBLE du sprite (les planches ont
  // souvent des lignes transparentes sous les pieds). Calcule une fois par monstre.
  static int s_footEnc=-1, s_footPad=0;
  if(s_footEnc!=g_enc){ s_footEnc=g_enc; s_footPad=0;
    for(int y=S.h-1;y>=0;--y){ bool any=false; for(int x=0;x<S.w;++x) if(S.px[y*S.w+x]!=spr::KEY){ any=true; break; } if(any) break; s_footPad++; }
    if(s_footPad>=S.h) s_footPad=0; }
  if(g_enc!=9) gb::draw_image_key(S.px,S.w,S.h,160-S.w/2,152-S.h+s_footPad,spr::KEY);   // pieds au sol (boss mine g_enc==9 : l'illustration CHEF.BMP montre deja l'orc)
  // ennemi : nom + barre PV
  gb::fill_rect(8,8,150,26,gb::rgb(20,16,10)); gb::fill_rect(8,8,150,2,gb::rgb(200,170,90));
  ui::gold(14,12,trc(M.name),true);
  int bw=140, f=(int)((mhp*bw)/(unsigned)M.VIT); gb::fill_rect(160,14,bw,10,gb::rgb(60,20,20)); gb::fill_rect(160,14,f,10,gb::rgb(190,60,50));
  // joueur : VIT/BL
  gb::fill_rect(158,176,154,56,gb::rgb(20,16,10)); gb::fill_rect(158,176,154,2,gb::rgb(200,170,90));
  ui::gold(164,179,player().name,true); char b[40];
  int pv=player().VIT-player().BL; if(pv<0)pv=0; int pbw=140, pf=(player().VIT>0)?(pv*pbw)/player().VIT:0;
  gb::fill_rect(164,194,pbw,8,gb::rgb(45,60,45)); gb::fill_rect(164,194,pf,8,gb::rgb(80,180,90));
  snprintf(b,40,"PV %d/%d",pv,player().VIT); ui::text(164,204,b,gb::rgb(230,220,180));
  snprintf(b,40,"Or %d  Niv %d",player().gold,player().level); ui::text(164,218,b,gb::rgb(220,200,120));
  if(phase==0){
    const char* A[4]={i18n::tr(i18n::CB_ATTACK),i18n::tr(i18n::CB_MAGIC),i18n::tr(i18n::CB_ITEM),i18n::tr(i18n::CB_FLEE)};
    gb::fill_rect(8,176,146,56,gb::rgb(20,16,10)); gb::fill_rect(8,176,146,2,gb::rgb(200,170,90));
    for(int i=0;i<4;i++){ if(i==sel){ gb::fill_rect(12,178+i*13,138,12,gb::rgb(60,45,20)); ui::gold(16,179+i*13,">",true);} ui::text(28,179+i*13,A[i],gb::rgb(235,225,190)); }
  } else if(phase==5){
    // BUG-02 : grille 2 colonnes x 3 lignes des sorts connus (pagination par 6)
    int K[data::SPELL_COUNT]; int n=known_list(K); int page=spellSel/6;
    gb::fill_rect(8,150,304,82,gb::rgb(20,16,10)); gb::fill_rect(8,150,304,2,gb::rgb(200,170,90));
    for(int k=0;k<6;k++){ int i=page*6+k; if(i>=n) break; int s=K[i];
      int cx=12+(k%2)*150, y=156+(k/2)*15; bool ok=player().mana>=spell_cost(s);
      if(i==spellSel){ gb::fill_rect(cx,y-2,146,13,gb::rgb(60,45,20)); ui::gold(cx+2,y-1,">",true); }
      ui::text(cx+12,y-1,trc(SPELL_SHORT[s]),ok?gb::rgb(235,225,190):gb::rgb(130,120,105));
      char c[8]; snprintf(c,8,"%d",spell_cost(s)); ui::text(cx+134-ui::text_w(c),y-1,c,gb::rgb(180,160,220)); }
    if(n>6){ char pg[24]; snprintf(pg,sizeof(pg),"%d/%d",page+1,(n+5)/6); ui::text(280,201,pg,gb::rgb(170,160,132)); }
    if(spellSel<n) ui::text(14,201,trc(SPELL_FX[K[spellSel]]),gb::rgb(200,190,160));
    char mm[28]; snprintf(mm,28,"%s %d/%d",i18n::tr(i18n::CB_POWER),player().mana,player().manaMax); ui::gold(14,216,mm,true);
    ui::text(196,216,i18n::tr(i18n::CB_CAST_H),gb::rgb(170,160,132));
  } else {
    gb::fill_rect(8,176,304,56,gb::rgb(20,16,10)); gb::fill_rect(8,176,304,2,gb::rgb(200,170,90));
    ui::text(16,190,trc(msg),gb::rgb(236,226,192)); ui::text(230,210,i18n::tr(i18n::CB_NEXT),gb::rgb(170,160,132));
  }
}
}
