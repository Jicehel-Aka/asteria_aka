// ASTERIA (Gamebuino AKA) - Combat tour par tour : armes manuelles, armures, fleches, critique AU, corde, objets & butin.
#include "scene/combat_scene.h"
#include "scene/world_scene.h"
#include "i18n.h"
#include "platform/gb_port.h"
#include "ui/text.h"
#include "player.h"
#include "gamestate.h"
#include "progress.h"
#include "content_i18n.h"
#include "audio/jingle.h"
#include "generated/asteria_monster.h"
#include "generated/asteria_spells_gen.h"
#include "generated/asteria_gen.h"
#include "generated/asteria_maps.h"
#include <cstdio>
#include <cstring>

namespace asteria {

static int g_enc=0;
static const char* g_bmp="/sdcard/ASTERIA/battle/FORE_J.BMP";
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
// L'arme est choisie dans Pause > Sac & Equipement ; si elle n'est plus possedee,
// repli automatique sur la meilleure. Les arcs tirent avec un bonus au toucher
// (attaque a distance) et consomment une fleche renforcee (+2 degats) s'il y en a.
// ---------------------------------------------------------------------------
struct WeaponDef{ const char* id; int bonus; int hit; bool bow; };
static const WeaponDef WEAPONS[]={
  {"OBJ_0037",0,0,false},   // Dague (depart) : 1D
  {"OBJ_0001",2,0,false},   // Epee usee      : 1D+2
  {"OBJ_0008",2,1,true },   // Arc court      : 1D+2, +1 toucher
  {"OBJ_0002",3,0,false},   // Hache          : 1D+3
  {"OBJ_0020",3,2,true },   // Arc de chasseur: 1D+3, +2 toucher
  {"OBJ_0032",4,1,false},   // Epee de fer    : 1D+4, +1 toucher
};
static const int WEAPON_N=(int)(sizeof(WEAPONS)/sizeof(WEAPONS[0]));

static int ring_bonus(){ int rg=item_index("OBJ_0040"); return (rg>=0 && player().inv.has(rg,1))?1:0; }

// Respecte l'arme choisie dans Sac & Equipement si possedee (repli sur la meilleure si invalide).
static const WeaponDef& active_weapon(){
  int cur = player().weapon;
  if(cur >= 0 && cur < (int)data::ITEM_COUNT){
    for(int i=0;i<WEAPON_N;i++){
      if(!strcmp(data::ITEMS[cur].id, WEAPONS[i].id) && (i==0 || player().inv.has(cur,1))){
        return WEAPONS[i];
      }
    }
  }
  int best=0;
  for(int i=1;i<WEAPON_N;i++){
    int it=item_index(WEAPONS[i].id);
    if(it>=0 && player().inv.has(it,1) &&
       WEAPONS[i].bonus+WEAPONS[i].hit >= WEAPONS[best].bonus+WEAPONS[best].hit){
      best=i;
    }
  }
  player().weapon = item_index(WEAPONS[best].id);
  return WEAPONS[best];
}

static int weapon_hit(){ return active_weapon().hit; }
static int weapon_dmg(bool& usedArrow){
  usedArrow = false;
  const WeaponDef& w = active_weapon();
  int d = rollN(1) + w.bonus + ring_bonus();
  int ar = item_index("OBJ_0027");
  if(w.bow && ar>=0 && player().inv.has(ar,1)){
    player().inv.remove(ar, 1);
    d += 2;
    usedArrow = true;
  }
  return d;
}

// ---------------------------------------------------------------------------
// Sorts (BUG-02) : les 12 sorts de data::SPELLS. Connus = masque player().spells
// (sorts de base des la creation) + sorts avances debloques par le niveau.
// Cout : 3 pouvoir pour un sort de base, po_cost pour un sort avance.
// Jet : 3D6 <= MA + niveau (le niveau aide). Effets adaptes au combat 1 contre 1.
// ---------------------------------------------------------------------------
// Noms courts (le menu est une grille 2 colonnes de ~140 px) ; traduits par trc().
static const char* const SPELL_SHORT[data::SPELL_COUNT]={
  "Attirance","Engourdissement","Dissipation","Repulsion","Soins","Acceleration",
  "Elementaire","Guerison majeure","Metamorphose","Invisibilite","Mimetisme","Vivacite"
};
// Effet resume affiche sous la grille des sorts (traduit par trc()).
static const char* const SPELL_FX[data::SPELL_COUNT]={
  "+2 au toucher pendant 3 tours","L'ennemi perd son prochain tour","1D+2 degats, annule ses ruses",
  "2D degats a la cible","Soigne 2D blessures","Tu rejoues avant l'ennemi","Un elementaire frappe 3 tours (2D)",
  "Soigne 4D blessures","Ours : +3 deg, +2 toucher, bouclier 3D","L'ennemi rate 2 attaques, fuite assuree",
  "-4 CO a l'ennemi pendant 3 tours","+3 toucher, +1 degat pendant 3 tours"
};

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

// Liste d'objets utilisables en combat (potions, miel, herbe)
static int combat_items(int* outItems){
  static const char* IDS[]={"OBJ_0033","OBJ_0005","OBJ_0026","OBJ_0034"};
  int n=0;
  for(int i=0;i<4;i++){
    int it=item_index(IDS[i]);
    if(it>=0 && player().inv.has(it,1)) outItems[n++]=it;
  }
  return n;
}

static void drop_zone_loot(char* out, int cap){
  out[0]=0;
  if(g_enc==9 || g_enc==19 || g_enc==24 || g_enc==25) return;
  if((d6()) > 3) return;   // 50% de chance de butin d'artisanat
  int loc = data::MAPS[world_scene().map].location;
  const char* id = nullptr;
  if(loc==8){ static const char* F[]={"OBJ_0026","OBJ_0036","OBJ_0034"}; id=F[d6()%3]; }
  else if(loc==9){ id = (d6()&1) ? "OBJ_0012" : "OBJ_0013"; }
  else if(loc==20||loc==23){ id = (d6()&1) ? "OBJ_0027" : "OBJ_0005"; }
  if(!id) return;
  int it = item_index(id);
  if(it>=0 && player().inv.add(it,1)){
    snprintf(out, cap, " (+%s)", trc(data::ITEMS[it].name));
  }
}

// Lance le sort s (deja paye et reussi) : applique l'effet et ecrit le message.
void CombatScene::cast(int s){
  switch(s){
    case 0:  hitBuff=3;  hitBonus=2; snprintf(msg,64,"Attirance : +2 au toucher (3 tours)"); break;
    case 1:  stunned=true; snprintf(msg,64,"Engourdissement reussi !"); break;
    case 2:  { int d=rollN(1)+2; hurt_monster(mhp,d); enemyDebuff=0; snprintf(msg,64,"Dissipation ! -%d",d); } break;
    case 3:  { int d=rollN(2); hurt_monster(mhp,d); snprintf(msg,64,"Repulsion ! -%d",d); } break;
    case 4:  { int h=rollN(2); heal_player(h); snprintf(msg,64,"Soins ! Blessures -%d",h); } break;
    case 5:  haste=true; snprintf(msg,64,"Acceleration : tu rejoues avant l'ennemi !"); break;
    case 6:  summon=3; snprintf(msg,64,"Un elementaire surgit (3 tours)"); break;
    case 7:  { int h=rollN(4); heal_player(h); snprintf(msg,64,"Guerison majeure ! Blessures -%d",h); } break;
    case 8:  bear=3; shield=rollN(3); snprintf(msg,64,"Metamorphose en ours ! (bouclier %d)",shield); break;
    case 9:  invis=2; snprintf(msg,64,"Invisibilite : l'ennemi frappe dans le vide"); break;
    case 10: enemyDebuff=3; snprintf(msg,64,"Mimetisme : l'ennemi se trompe (-4 CO)"); break;
    case 11: hitBuff=3; hitBonus=3; dmgBonus=1; snprintf(msg,64,"Vivacite : +3 toucher, +1 degat (3 tours)"); break;
  }
}

static int itemSel = 0;

void CombatScene::enter(){
  phase=0; sel=0; result=0; g_rng=gb::millis()|1u; mhp=(unsigned)mon::MON[g_enc].VIT; msg[0]=0;
  stunned=false; spellSel=0; itemSel=0; hitBuff=0; hitBonus=0; dmgBonus=0; summon=0; bear=0; shield=0;
  invis=0; enemyDebuff=0; haste=false; summonActed=false;
}

void CombatScene::update(SceneManager& m){
  uint32_t p=gb::buttons_pressed();
  const mon::Stats& M=mon::MON[g_enc];
  if(phase==0){
    if(p&gb::BTN_DOWN) sel=(sel+1)%4;
    if(p&gb::BTN_UP)   sel=(sel+3)%4;
    if(p&gb::BTN_A){
      if(sel==0){
        int r=roll3();
        int toHit=player().CO+2*ring_bonus()+weapon_hit()+(hitBuff>0?hitBonus:0)+(bear>0?2:0);
        if(r<=toHit){
          bool usedArrow=false;
          bool crit = (r <= 3 + player().AU / 5);
          lastDmg=weapon_dmg(usedArrow)+(hitBuff>0?dmgBonus:0)+(bear>0?3:0)+(crit?2:0);
          hurt_monster(mhp,lastDmg);
          snprintf(msg,64,"%s%s (%d<=%d) -%d",
                   crit?"Critique ! ":"Touche ! ",
                   usedArrow?"[Fleche]":"", r, toHit, lastDmg);
          audio::sfx_hit();
        } else {
          snprintf(msg,64,"Manque ! (%d>%d)",r,toHit);
        }
        phase=1;
      } else if(sel==3){
        int r=roll3();
        int sr=item_index("OBJ_0025"), cr=item_index("OBJ_0004");
        if(invis>0 || r<=player().IN){
          snprintf(msg,64,"Tu prends la fuite."); result=3; phase=3;
        } else if(sr>=0 && player().inv.has(sr,1)){
          snprintf(msg,64,"Corde solide : fuite assuree !"); result=3; phase=3;
        } else if(cr>=0 && player().inv.has(cr,1)){
          player().inv.remove(cr,1);
          snprintf(msg,64,"Corde lancee : fuite assuree !"); result=3; phase=3;
        } else {
          snprintf(msg,64,"Fuite ratee ! (%d>IN)",r); phase=1;
        }
      } else if(sel==2){
        int ci[4]; int cn=combat_items(ci);
        if(cn==0){ snprintf(msg,64,"Aucun objet de soin !"); phase=4; }
        else { itemSel=0; phase=6; }
      } else {
        spellSel=0; phase=5;
      }
    }
  } else if(phase==1){   // apres action joueur -> mort du monstre ? sinon tour ennemi
    if(p&gb::BTN_A){
      if(mhp>0 && summon>0 && !summonActed){
        summonActed=true; summon--; int d=rollN(2); hurt_monster(mhp,d);
        snprintf(msg,64,"L'elementaire frappe ! -%d",d); return;
      }
      if(mhp==0){
        int xp=M.VIT/2+M.CO; int gold=M.VIT/4+d6(); player().xp+=xp; player().gold+=gold;
        int lvl0=player().level;
        while(player().xp>=player().level*100){
          player().xp-=player().level*100; player().level++;
          player().VIT+=5; player().manaMax+=2;
          player().BL-=5; if(player().BL<0) player().BL=0;
          player().mana+=2; if(player().mana>player().manaMax) player().mana=player().manaMax;
        }
        if(g_enc==9){ flag_set("boss_mine");
          // Butin de boss : Clef du Puits (ouvre la Route) + Pierre noire gravee (fil rouge de l'Arc).
          int kp=item_index("OBJ_0015"), pn=item_index("OBJ_0014"), ca=item_index("OBJ_0010");
          if(kp>=0) player().inv.add(kp,1); if(pn>=0) player().inv.add(pn,1);
          if(ca>=0 && !player().inv.has(ca,1)) player().inv.add(ca,1);   // casque de mineur (Q1 Doran)
          complete_quest_reward(6);
          snprintf(msg,64,"Chef Gobelin vaincu ! Clef du Puits + Pierre noire");
        } else if(g_enc==19){ flag_set("boss_gardien"); snprintf(msg,64,"Le Gardien du Seuil tombe."); }
        else if(g_enc==24){ flag_set("boss_coeur"); snprintf(msg,64,"Le Coeur de Haine est detruit !"); }
        else if(g_enc==25){ flag_set("boss_corbeau"); complete_quest_reward(11); snprintf(msg,64,"Corbeau Noir tombe ! La route est libre."); }
        else {
          char loot[32]; drop_zone_loot(loot, sizeof(loot));
          if(player().level>lvl0) snprintf(msg,64,"Victoire ! Niveau %d%s",player().level,loot);
          else snprintf(msg,64,"Victoire ! +%d XP +%d or%s",xp,gold,loot);
        }
        if(g_enc!=9 && g_enc!=25) audio::play(player().level>lvl0 ? audio::TUNE_LEVELUP : audio::TUNE_VICTORY);   // boss : jingle joue par complete_quest_reward
        result=1; phase=3;
      } else if(haste){ haste=false; snprintf(msg,64,"Acceleration : a toi de jouer !"); phase=2; }
      else if(stunned){ stunned=false; snprintf(msg,64,"%s est engourdi.",M.name); phase=2; }
      else if(invis>0){ invis--; snprintf(msg,64,"%s frappe dans le vide !",M.name); phase=2; }
      else {
        int co=M.CO-(enemyDebuff>0?4:0); int r=roll3();
        if(r<=co){
          int dmg=rollN(M.BLd);
          int def=armor_defense(player());
          if(def>0){ dmg = (dmg>def) ? (dmg-def) : 1; }
          if(shield>0){ int a=dmg<shield?dmg:shield; shield-=a; dmg-=a; }   // Metamorphose : la peau d'ours encaisse
          player().BL+=dmg;
          if(def>0) snprintf(msg,64,"%s attaque ! -%d (armure -%d)",M.name,dmg,def);
          else snprintf(msg,64,"%s attaque ! -%d",M.name,dmg);
          audio::sfx_hurt();
          if(player().BL>=player().VIT){ result=2; phase=3; } else phase=2;
        } else {
          snprintf(msg,64,"%s te manque. (%d>CO)",M.name,r); phase=2;
        }
      }
      if(phase==2){ if(hitBuff>0) hitBuff--; if(bear>0) bear--; if(enemyDebuff>0) enemyDebuff--; }   // fin de tour : les effets s'usent
    }
  } else if(phase==2 || phase==4){
    if(p&gb::BTN_A){ phase=0; summonActed=false; }
  } else if(phase==5){
    int K[data::SPELL_COUNT]; int n=known_list(K); if(n==0){ phase=0; return; }
    if(spellSel>=n) spellSel=n-1;
    // grille 2 colonnes x 3 lignes (pages de 6) : haut/bas = ligne, gauche/droite = colonne
    if(p&gb::BTN_DOWN)  spellSel=(spellSel+2<n)?spellSel+2:spellSel;
    if(p&gb::BTN_UP)    spellSel=(spellSel>=2)?spellSel-2:spellSel;
    if(p&gb::BTN_RIGHT) spellSel=(spellSel+1<n)?spellSel+1:spellSel;
    if(p&gb::BTN_LEFT)  spellSel=(spellSel>0)?spellSel-1:0;
    if(spellSel>=n) spellSel=n-1;
    if(p&gb::BTN_B) phase=0;
    if(p&gb::BTN_A){
      int s=K[spellSel]; int cost=spell_cost(s);
      if(player().mana<cost){ snprintf(msg,64,"Pas assez de pouvoir !"); phase=4; }
      else {
        player().mana-=cost; int r=roll3();
        if(r<=player().MA+player().level){ cast(s); audio::sfx_spell(); }
        else snprintf(msg,64,"Le sort echoue ! (%d>MA)",r);
        phase=1;
      }
    }
  } else if(phase==6){
    int ci[4]; int cn=combat_items(ci);
    if(cn==0){ phase=0; return; }
    if(itemSel>=cn) itemSel=cn-1;
    if(p&gb::BTN_DOWN) itemSel=(itemSel+1)%cn;
    if(p&gb::BTN_UP)   itemSel=(itemSel+cn-1)%cn;
    if(p&gb::BTN_B){ phase=0; return; }
    if(p&gb::BTN_A){
      char out[64]={0};
      if(use_item_field(player(), ci[itemSel], out, sizeof(out))){
        strncpy(msg, out, 63); msg[63]=0;
        phase=1;
      } else {
        strncpy(msg, out[0]?out:"Inutilisable.", 63); msg[63]=0;
        phase=4;
      }
    }
  } else {
    if(p&gb::BTN_A){ if(result==2){ m.set(SceneId::GAMEOVER); } else m.set(g_return); }
  }
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
  snprintf(b,40,"PV %d/%d  PM %d",pv,player().VIT,player().mana); ui::text(164,204,b,gb::rgb(230,220,180));
  const WeaponDef& aw = active_weapon();
  int wi = item_index(aw.id);
  snprintf(b,40,"Niv %d %s",player().level,(wi>=0)?trc(data::ITEMS[wi].name):"");
  ui::text(164,218,b,gb::rgb(220,200,120));

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
  } else if(phase==6){
    int ci[4]; int cn=combat_items(ci);
    gb::fill_rect(8,146,304,86,gb::rgb(20,16,10)); gb::fill_rect(8,146,304,2,gb::rgb(200,170,90));
    ui::gold(14,150,trc("Sac (soins)"),true);
    for(int i=0;i<cn;i++){
      int y=166+i*14;
      if(i==itemSel){ gb::fill_rect(12,y-2,296,13,gb::rgb(60,45,20)); ui::gold(14,y-1,">",true); }
      int idx = player().inv.find(ci[i]); int qty = idx>=0?(int)player().inv.slots[idx].qty:0;
      char lb[48]; snprintf(lb,sizeof(lb),"%s x%d",trc(data::ITEMS[ci[i]].name),qty);
      ui::text(26,y-1,lb,gb::rgb(235,225,190));
    }
    if(itemSel<cn) ui::text(110,150,trc(item_desc(ci[itemSel])),gb::rgb(175,165,135));   // description de l'objet choisi (une ligne)
    ui::text(190,218,trc("A: utiliser   B: retour"),gb::rgb(170,160,132));
  } else {
    gb::fill_rect(8,176,304,56,gb::rgb(20,16,10)); gb::fill_rect(8,176,304,2,gb::rgb(200,170,90));
    ui::text(16,190,trc(msg),gb::rgb(236,226,192)); ui::text(230,210,i18n::tr(i18n::CB_NEXT),gb::rgb(170,160,132));
  }
}
}
