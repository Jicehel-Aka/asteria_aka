// ASTERIA (Gamebuino AKA) - Scene monde : exploration, PNJ, cueillette (herbes/champignons/miel/bois),
// fenetres "Objet obtenu" et Personnage (ui/item_ui, scene/char_window),
// menu Pause complet (Personnage, Alchimie & Soins, Journal & Stats, 3 emplacements de sauvegarde).
#include <cstdio>
#include <cstring>
#include "i18n.h"
#include "scene/world_scene.h"
#include "daytime.h"
#include "save.h"
#include "scene/dungeon_scene.h"
#include "scene/combat_scene.h"   // spell_known / spell_cost (soins hors combat)
#include "ui/text.h"
#include "platform/gb_port.h"
#include "generated/asteria_maps.h"
#include "generated/asteria_gen.h"
#include "generated/asteria_hero.h"
#include "generated/asteria_tiles.h"
#include "audio/jingle.h"
#include "autotile.h"
#include "content_i18n.h"
#include "generated/asteria_water.h"
#include "generated/asteria_npc.h"
#include "generated/asteria_dialog.h"
#include "gamestate.h"
#include "generated/asteria_quest.h"
#include "generated/asteria_event.h"
#include "generated/asteria_recipes_gen.h"
#include "player.h"
#include "progress.h"
#include "ui/item_ui.h"
#include "scene/char_window.h"
#include "scene/journal.h"
#include "scene/worldmap.h"
#include "scene/dialog_choice.h"

namespace asteria {
using namespace data;

// Verrou de passage : renvoie l'indice de blocage (ou nullptr si autorisé).
// Trame validée (GDD v4) : la Forêt exige la carte (vu_maire) ; la Mine exige
// la piste (piste_mine, fin Q5) ; la Route Royale exige la Clef du Puits + niv.5.
static const char* gate_block(int target_loc){
  if(target_loc==8 && !flag_get("vu_maire"))
    return "Tu devrais d'abord voir le maire Aldren : il te remettra la carte.";
  if(target_loc==9 && !flag_get("piste_mine"))
    return "Rien ne t'indique encore où chercher. Enquête d'abord dans la forêt.";
  if(target_loc==20 && (!player().inv.has(item_index("OBJ_0015"),1) || player().level<5))
    return "La voie est coupée : il te faut la Clef du Puits et plus d'expérience (niv. 5).";
  if(target_loc==10 && !flag_get("boss_mine"))
    return "La route vers Grand-Castel longe la Vieille Mine. Nettoie-la d'abord de sa corruption.";
  if(target_loc==21 && quest_status(7)!=2)
    return "Une porte scellée des Ténèbres. Fais d'abord parler la Pierre noire à Maître Albin.";
  return nullptr;
}

// couleurs de tuiles provisoires (avant sprites) : herbe,chemin,mur,eau,porte,pont,arbre,toit
static const gb::Color TCOL(uint8_t t){
  switch(t){ case 0:return gb::rgb(64,104,56); case 1:return gb::rgb(150,120,80);
    case 2:return gb::rgb(96,96,104); case 3:return gb::rgb(48,96,168); case 4:return gb::rgb(120,80,40);
    case 5:return gb::rgb(130,96,60); case 6:return gb::rgb(34,64,40); default:return gb::rgb(150,60,50);}
}

struct ChestDef { int loc,x,y,item,gold; const char* flag; int complete_q; const char* setflag; };
static const ChestDef CHESTS[]={
  {8,22,7,4,8,"coffre_foret",5,"piste_mine"},
  {0,3,21,2,5,"coffre_valbois",-1,nullptr},
  {20,4,16,-1,0,"coffre_carnet",10,"caravane_ok"},
  // maisons de Valbois (interieurs)
  {4,12,8,4,10,"coffre_maire",-1,nullptr},      // potion + 10 or
  {5,1,7,25,3,"coffre_milo",-1,nullptr},        // miel sauvage
  {6,10,7,12,5,"coffre_edric",-1,nullptr},      // lingot rouille (forge)
  {7,8,5,45,6,"coffre_chaumiere",-1,nullptr}    // Anneau de vigueur (v1.5)
};
static const int NCHEST=(int)(sizeof(CHESTS)/sizeof(CHESTS[0]));
static int chest_at(int map,int x,int y){
  int loc=MAPS[map].location;
  for(int i=0;i<NCHEST;i++) if(CHESTS[i].loc==loc && CHESTS[i].x==x && CHESTS[i].y==y && !flag_get(CHESTS[i].flag)) return i;
  return -1;
}

// Coffre ferme facon pixel-art (ref. fournie) : couvercle bombe ambre, 2 ferrures
// argent a rivets, contour sombre, plaque de serrure centrale. Dessine a (cx,cy).
static void draw_chest_closed(int cx,int cy){
  const uint16_t OL=gb::rgb(74,32,30), LA=gb::rgb(242,176,70), LM=gb::rgb(216,134,46),
    BR=gb::rgb(152,98,48), BD=gb::rgb(116,70,34),
    SV=gb::rgb(200,200,210), SD=gb::rgb(150,150,164), SH=gb::rgb(234,234,242), GD=gb::rgb(232,204,114);
  int bx=cx+1, by=cy+1;
  auto P=[&](int x,int y,uint16_t c){ gb::pixel(bx+x,by+y,c); };
  auto span=[&](int y,int&a,int&b){ if(y==1){a=3;b=10;} else if(y==2){a=2;b=11;} else {a=1;b=12;} };
  for(int y=1;y<=13;y++){ int a,b; span(y,a,b);
    for(int x=a;x<=b;x++){ uint16_t c = (y<=6)? ((y==2||y==4)?LA:LM) : ((x&1)?BR:BD); P(x,y,c); } }
  for(int y=1;y<=13;y++){ int a,b; span(y,a,b); P(a-1,y,OL); P(b+1,y,OL); }   // cotes
  for(int x=3;x<=10;x++) P(x,0,OL);   // crown
  for(int x=1;x<=12;x++){ P(x,6,OL); P(x,13,OL); }   // jointure + bas
  for(int bX=3; bX<=9; bX+=6){   // 2 ferrures argent
    for(int y=1;y<=12;y++){ if(y==6){P(bX,y,SH);P(bX+1,y,SH);continue;} P(bX,y,SD); P(bX+1,y,SV); }
    for(int y=2;y<=11;y+=3) P(bX,y,SH); }   // rivets
  gb::fill_rect(bx+6,by+7,3,4,SV); P(6,7,SD);P(8,7,SD);P(6,10,SD);P(8,10,SD);   // plaque serrure
  P(7,8,OL); P(7,9,GD);   // trou de serrure
  P(5,3,LA); P(6,3,LA);   // reflet couvercle
}

static int map_for_loc(int loc);   // fwd map_for_loc
static bool door_leads_somewhere(const Map& M,int x,int y){
  for(int i=0;i<M.portal_n;++i) if(M.portals[i].x==x&&M.portals[i].y==y) return map_for_loc(M.portals[i].target_loc)>=0;
  return false;
}
static int npc_at(const Map& M,int x,int y){
  for(int i=0;i<M.spawn_n;++i) if(M.spawns[i].x==x&&M.spawns[i].y==y) return M.spawns[i].npc;
  return -1;
}
static int pick_node(const dlg::NpcDialog& D){
  for(int i=0;i<D.nnodes;i++){
    const char* r=D.nodes[i].require; bool ok;
    if(!r||!r[0]) ok=true; else if(r[0]=='!') ok=!flag_get(r+1); else ok=flag_get(r);
    if(ok) return i;
  }
  return D.nnodes>0?D.nnodes-1:0;
}

// BUG-06 : PRNG xorshift32 independant de l'horloge CPU pour les rencontres aleatoires
bool g_no_encounters=false;   // mis a true par les harnais de validation (questcheck) ; jeu reel = false
static uint32_t ws_rng=0u;
static inline uint32_t ws_rand(){ if(!ws_rng) ws_rng=(gb::millis()|1u); ws_rng^=ws_rng<<13; ws_rng^=ws_rng>>17; ws_rng^=ws_rng<<5; return ws_rng; }
static int d6w(){ return (int)(ws_rand()%6)+1; }

// Nom affichable d'un lieu (bandeau HUD) ; trc() le traduit si une entree existe, sinon FR.
// Fond de combat selon le biome et l'heure (12 decors jour/nuit fournis dans battle/).
static const char* backdrop(int loc){
  bool n=daytime::is_night();
  switch(loc){
    case 8:           return n?"/sdcard/ASTERIA/battle/FORE_N.BMP":"/sdcard/ASTERIA/battle/FORE_J.BMP";
    case 20: case 22: return n?"/sdcard/ASTERIA/battle/ROUT_N.BMP":"/sdcard/ASTERIA/battle/ROUT_J.BMP";
    case 23:          return n?"/sdcard/ASTERIA/battle/VALL_N.BMP":"/sdcard/ASTERIA/battle/VALL_J.BMP";
    case 9:           return "/sdcard/ASTERIA/battle/MINE_L.BMP";
    case 21:          return "/sdcard/ASTERIA/battle/TEMP.BMP";
    default:          return n?"/sdcard/ASTERIA/battle/VALB_N.BMP":"/sdcard/ASTERIA/battle/VALB_J.BMP";
  }
}
// Zones a ciel ouvert : soumises au cycle jour / nuit (les interieurs restent eclaires).
static bool is_outdoor(int loc){
  switch(loc){ case 0: case 8: case 9: case 10: case 20: case 22: case 23: return true; default: return false; }
}
static const char* loc_name(int loc){
  switch(loc){
    case 0:  return "Valbois";
    case 1:  return "Taverne de Valbois";
    case 2:  return "Forge de Doran";
    case 3:  return "Échoppe de Mira";
    case 4:  return "Maison du maire";
    case 5:  return "Maison de Milo";
    case 6:  return "Atelier d'Edric";
    case 7:  return "Chaumière abandonnée";
    case 8:  return "Forêt de Brume";
    case 9:  return "Vieille Mine";
    case 10: return "Grand-Castel";
    case 20: return "Route Royale";
    case 21: return "Antre Maléfique";
    case 22: return "Grande Plaine";
    case 23: return "Vallée des Pierres";
    default: return "";
  }
}

WorldScene& world_scene(){ static WorldScene s; return s; }
void new_game_reset(){
  daytime::reset();                                   // nouvelle partie : 8h du matin
  WorldScene& w=world_scene(); w.started=false; w.confirmQuit=false;
  w.pauseSel=0; w.pauseSub=0; w.pauseListSel=0; w.stepCount=0;
  w.pauseMsg=nullptr; w.epiLine=0; w.notice=false; w.dlg=false; w.savedMap=-2;
  item_ui::pickup_clear(); charwin::close(); journal::close(); worldmap::close(); choice::reset();
  state_reset(); progress_reset(); player()=new_caithness(); dungeon_reset();
}

static int map_for_loc(int loc){ for(int i=0;i<MAP_COUNT;++i) if(MAPS[i].location==loc) return i; return -1; }
void WorldScene::load_map(int m){ camMap=-1;   // recadrage camera sur la nouvelle carte
  map=m; px=MAPS[m].px; py=MAPS[m].py; gb::log(MAPS[m].id); }

void WorldScene::enter(){
  if(started) return;   // retour de combat : on garde la carte/position (et savedMap -> pas de re-sauvegarde)
  started=true; int gp=map_for_loc(22); load_map(gp>=0?gp:0);   // Arc 0 : demarrage dans la Grande Plaine
  if(MAPS[map].location==22){
    quest_start(17); discover_zone(22); notice=true;
    noticeTxt="La Brèche se referme derrière toi. Te voici dans la Grande Plaine d'Asteria.";
  }
}

// ---- Menu Pause enrichi (pSub: 0 Principal, 1 Sac & Equip., 2 Alchimie & Soins, 3 Journal & Stats, 4 Sauvegarde 3 slots) ----
static char g_pmsg_buf[72]={0};
// Resumes des 3 emplacements : lus sur la SD a l'OUVERTURE du sous-menu et apres
// une sauvegarde, jamais a chaque image (3 lectures SD par frame figeaient la console).
static SaveSummary g_slotSum[3];
static void refresh_slot_summaries(){ for(int s=1;s<=3;s++) save_slot_summary(s,g_slotSum[s-1]); }

static int alch_entries(int* kinds, int* ids){
  // kinds: 0=Sort Soins(4), 1=Sort Guerison majeure(7), 2=Recette alchimie portable (station==-1)
  int n=0;
  if(spell_known(4)){ kinds[n]=0; ids[n]=4; n++; }
  if(spell_known(7)){ kinds[n]=1; ids[n]=7; n++; }
  for(int i=0;i<RECIPE_COUNT;i++){
    if(RECIPES[i].station==-1){ kinds[n]=2; ids[n]=i; n++; }
  }
  return n;
}


// Quete suivie (bandeau, etoile de la carte) : la premiere quete principale en cours,
// sinon la premiere secondaire en cours ; -1 s'il n'y en a aucune.
static int hud_quest(){
  for(int pass=0;pass<2;pass++)
    for(int i=0;i<quest::QUEST_COUNT;i++) if(quest_status(i)==1 && quest::QUEST[i].main==(pass==0)) return i;
  return -1;
}

void WorldScene::pause_update(SceneManager& mgr,uint32_t p){
  if(p&gb::BTN_MENU){ confirmQuit=false; pauseSub=0; pauseMsg=nullptr; return; }

  if(pauseSub==0){
    const int N=7;
    if(p&gb::BTN_DOWN){ pauseSel=(pauseSel+1)%N; pauseMsg=nullptr; audio::sfx_select(); }
    if(p&gb::BTN_UP){   pauseSel=(pauseSel+N-1)%N; pauseMsg=nullptr; audio::sfx_select(); }
    if(p&gb::BTN_B){ confirmQuit=false; return; }
    if(!(p&gb::BTN_A)) return;
    switch(pauseSel){
      case 0: confirmQuit=false; break;
      case 1: charwin::open(0); pauseMsg=nullptr; break;   // fenetre personnage par-dessus la pause
      case 2: journal::open(); pauseMsg=nullptr; break;   // journal de quetes par-dessus la pause
      case 3: worldmap::open(hud_quest()); pauseMsg=nullptr; break;   // carte du monde (etoile : objectif de la quete du bandeau)
      case 4: pauseSub=2; pauseListSel=0; pauseMsg=nullptr; break;
      case 5: pauseSub=4; pauseListSel=active_save_slot()-1; pauseMsg=nullptr; refresh_slot_summaries(); break;
      case 6: mgr.set(SceneId::TITLE); break;
    }
    return;
  }

  if(p&gb::BTN_B){ pauseSub=0; pauseMsg=nullptr; return; }

  if(pauseSub==2){ // Alchimie & Soins
    int kinds[8], ids[8]; int n=alch_entries(kinds,ids);
    if(n>0 && pauseListSel>=n) pauseListSel=n-1;
    if(n>0 && (p&gb::BTN_DOWN)){ pauseListSel=(pauseListSel+1)%n; pauseMsg=nullptr; }
    if(n>0 && (p&gb::BTN_UP)){   pauseListSel=(pauseListSel+n-1)%n; pauseMsg=nullptr; }
    if(n>0 && (p&gb::BTN_A)){
      int k=kinds[pauseListSel], id=ids[pauseListSel];
      Character& c=player();
      if(k==0 || k==1){
        int cost=spell_cost(id);
        if(c.BL<=0){ pauseMsg="Tu n'as aucune blessure."; audio::sfx_error(); }
        else if(c.mana<cost){ pauseMsg="Pas assez de pouvoir !"; audio::sfx_error(); }
        else {
          c.mana-=cost; int h=0; for(int i=0;i<(id==4?2:4);i++) h+=d6w();
          c.BL-=h; if(c.BL<0)c.BL=0;
          snprintf(g_pmsg_buf,sizeof(g_pmsg_buf),trc("%s : -%d blessures."),trc(id==4?"Soins":"Guérison"),h);
          pauseMsg=g_pmsg_buf; audio::sfx_spell();
        }
      } else {
        if(try_craft(c, id, false)){
          snprintf(g_pmsg_buf,sizeof(g_pmsg_buf),trc("Préparé : %s !"),trc(ITEMS[RECIPES[id].out_item].name));
          pauseMsg=g_pmsg_buf; audio::sfx_craft();
        } else {
          pauseMsg="Ingrédients insuffisants."; audio::sfx_error();
        }
      }
    }
  } else if(pauseSub==4){ // Choisir Slot Sauvegarde 1..3
    if(p&gb::BTN_DOWN) pauseListSel=(pauseListSel+1)%3;
    if(p&gb::BTN_UP)   pauseListSel=(pauseListSel+2)%3;
    if(p&gb::BTN_A){
      int sl=pauseListSel+1;
      set_active_save_slot(sl);
      if(game_save_slot(sl)){
        refresh_slot_summaries();
        snprintf(g_pmsg_buf,sizeof(g_pmsg_buf),trc("Sauvegardé (Empl. %d) !"),sl);
        audio::sfx_save();
      } else {
        snprintf(g_pmsg_buf,sizeof(g_pmsg_buf),"%s",trc("Échec sauvegarde SD."));
      }
      pauseMsg=g_pmsg_buf;
    }
  }
}

void WorldScene::update(SceneManager& mgr){
  const Map& M=MAPS[map]; uint32_t p=gb::buttons_pressed();
  if(MAPS[map].location==0 && !flag_get("valbois_intro")){ flag_set("valbois_intro"); quest_start(0); discover_zone(0);
    if(quest_status(17)==1) complete_quest_reward(17); notice=true; noticeTxt=evt::EVENTS[0].desc; }
  if(MAPS[map].location==10 && !flag_get("gc_intro")){ flag_set("gc_intro"); quest_start(8); notice=true; noticeTxt="Les hautes tours de Grand-Castel se dressent devant toi."; }
  if(MAPS[map].location==20 && !flag_get("route_intro")){ flag_set("route_intro"); quest_start(9); notice=true; noticeTxt="La Route Royale s'ouvre devant toi. Des marchands auraient disparu sur ses chemins..."; }
  if(MAPS[map].location==21 && !flag_get("antre_intro")){ flag_set("antre_intro"); if(quest_status(14)==0) quest_start(14); notice=true; noticeTxt="Tu pénètres dans l'Antre Maléfique. L'air brûle et pue l'acide."; }
  if(MAPS[map].location==23 && !flag_get("vallee_intro")){ flag_set("vallee_intro"); notice=true; noticeTxt="La Vallée des Pierres Murmurantes. Des monolithes gravés veillent entre les rochers."; }
  if(quest_status(0)==2 && quest_status(2)==0) quest_start(2);
  if(quest_status(2)==2 && quest_status(3)==0) quest_start(3);   // Q3 Les empreintes (annexe)
  if(charwin::is_open()){ charwin::update(p); return; }
  if(journal::is_open()){ journal::update(p); return; }
  if(worldmap::is_open()){ worldmap::update(p); return; }   // carte du monde (ouverte depuis la pause)   // journal de quetes (ouvert depuis la pause)   // fenetre personnage (B : retour au jeu ou au menu pause)
  if(confirmQuit){ pause_update(mgr,p); return; }   // menu pause : Personnage, Alchimie & Soins, Journal, Sauvegarde (3 empl.), Quitter
  if(notice){ if(p&(gb::BTN_A|gb::BTN_B)) notice=false; return; }
  if(started && !dlg && map!=savedMap){ savedMap=map; if(g_autosave) game_save(); }   // auto-sauvegarde a chaque arrivee sur une carte

  // ---- Final de l'Arc I : l'Antre Malefique (loc 21) ----
  if(MAPS[map].location==21){
    if(flag_get("boss_coeur")){   // le tyran est vaincu -> denouement
      if(!flag_get("fin_arc1")){
        static const char* D[]={
          "Le Cœur de Haine s'effondre dans un dernier spasme.",
          "Une lumière pâle revient : les Ténèbres refluent de la région.",
          "Les disparus perdus ne reviendront pas tous... mais le pays respire.",
          "Sur l'autel, un pendentif d'hermine. Tu le reconnais aussitôt.",
          "C'est celui de ta mère. Elle est passée ici, vivante.",
          "La piste des tiens mène vers l'Est, au-delà des Monts. (Fin de l'Arc I)" };
        if(epiLine<6){ notice=true; noticeTxt=D[epiLine++]; return; } else { if(quest_status(14)==1) complete_quest_reward(14); flag_set("fin_arc1"); }
      }
    } else {
      bool atAltar=(py<=5 && px>=8 && px<=12);
      if(atAltar){
        if(!flag_get("boss_gardien")){
          if(!flag_get("vu_gardien")){ flag_set("vu_gardien"); notice=true; noticeTxt="Le Gardien du Seuil, lieutenant du Cœur de Haine, surgit !"; return; }
          set_combat_return(SceneId::WORLD); start_combat(19,"/sdcard/ASTERIA/battle/TEMP.BMP"); mgr.set(SceneId::COMBAT); return; }
        if(!flag_get("vu_coeur")){ flag_set("vu_coeur"); notice=true; noticeTxt="Au centre, une chose sombre bat comme un cœur. Les veinules rouges palpitent."; return; }
        set_combat_return(SceneId::WORLD); start_combat(24,"/sdcard/ASTERIA/battle/TEMP_A.BMP"); mgr.set(SceneId::COMBAT); return;   // coeur du temple
      }
    }
  }

  if(dlg){ const dlg::Node& N=dlg::DIALOG[dlgNpc].nodes[dlgNode];
    // effets du noeud (drapeau, objet, quete) : en fin de lignes, ou apres un choix accepte
    auto apply=[&](){ if(N.set_flag) flag_set(N.set_flag); if(N.give_item>=0 && player().inv.add(N.give_item,1)) item_ui::pickup_push(N.give_item,1); if(N.complete_quest>=0) complete_quest_reward(N.complete_quest); if(N.start_quest>=0) quest_start(N.start_quest); };
    if(choice::active()){ choice::Result r=choice::update(p); if(r!=choice::RUNNING){ if(r==choice::DONE_ACCEPT) apply(); dlg=false; } return; }
    if(p&gb::BTN_A){ dlgLine++; if(dlgLine>=N.nlines){ if(choice::begin(dlgNpc,dlgNode)) return;   // dialogue a choix (attitudes)
        apply(); dlg=false; } }
    if(p&gb::BTN_B) dlg=false; return; }
  if(item_ui::pickup_active()){ item_ui::pickup_update(p); return; }   // fenetres "Objet obtenu" en attente

  int nx=px,ny=py;
  if(p&gb::BTN_UP){ny=py-1;dir=1;} if(p&gb::BTN_DOWN){ny=py+1;dir=0;}
  if(p&gb::BTN_LEFT){nx=px-1;dir=2;} if(p&gb::BTN_RIGHT){nx=px+1;dir=3;} if(p&(gb::BTN_UP|gb::BTN_DOWN|gb::BTN_LEFT|gb::BTN_RIGHT)) lastMove=gb::millis();

  if(nx!=px||ny!=py){
    if(nx>=0&&ny>=0&&nx<M.w&&ny<M.h){
      uint8_t t=M.tiles[ny*M.w+nx];
      if(!TILE_SOLID[t] && npc_at(M,nx,ny)<0 && chest_at(map,nx,ny)<0 && !(t==4 && !door_leads_somewhere(M,nx,ny))){
        px=nx; py=ny; stepCount++; bool _tp=false;
        if((stepCount % 25) == 0 && player().mana < player().manaMax){ player().mana++; }

        if(MAPS[map].location==8){   // cueillette (19 points) : 9 herbes, 4 champignons, 3 miels, 3 x2 bois
          static const int PKX[]={31,34,7,25,30,16,12,8,23, 4,33,13,26, 11,24,19, 12,26,20};
          static const int PKY[]={24,24,14,8,8,6,29,29,31, 14,8,6,31, 10,18,26, 11,19,27};
          // type: 0=herbe(33), 1=champignon(34), 2=miel(25), 3=bois(35, x2)
          static const uint8_t PKT[]={0,0,0,0,0,0,0,0,0, 1,1,1,1, 2,2,2, 3,3,3};
          static const char* PKF[]={"pk0","pk1","pk2","pk3","pk4","pk5","pk6","pk7","pk8","pk9","pk10","pk11","pk12","pk13","pk14","pk15","pk16","pk17","pk18"};
          for(int i=0;i<19;i++) if(px==PKX[i]&&py==PKY[i]&&!flag_get(PKF[i])){
            int kind=PKT[i];
            const char* id = (kind==0)?"OBJ_0034":(kind==1)?"OBJ_0035":(kind==2)?"OBJ_0026":"OBJ_0036";
            int qty = (kind==3) ? 2 : 1;
            int it=item_index(id);
            if(it>=0 && player().inv.add(it,qty)){ flag_set(PKF[i]); item_ui::pickup_push(it,qty); }   // fenetre "Objet obtenu"
            else { notice=true; noticeTxt="Ton sac est plein !"; audio::sfx_error(); break; }   // la cueillette reste en place
            audio::sfx_pickup();
            break;
          }
        }

        for(int i=0;i<M.portal_n;++i){ const Portal& pt=M.portals[i];
          if(pt.x==px&&pt.y==py){
            const char* blk=gate_block(pt.target_loc);
            if(blk){ notice=true; noticeTxt=blk; break; }   // verrou : indice, pas de passage
            if(pt.target_loc==9){ discover_zone(9); audio::sfx_door(); mgr.set(SceneId::DUNGEON); return; }   // la mine est un donjon 1re personne
            int nm=map_for_loc(pt.target_loc);
            if(nm>=0){ load_map(nm); discover_zone(pt.target_loc); if(pt.sx>=0){px=pt.sx;py=pt.sy;} _tp=true; audio::sfx_door(); }
            else gb::log("WORLD: portail (zone sans carte encore)");
          }
        }

        { int _loc=MAPS[map].location;
          if(!_tp && _loc==9 && py<=3 && !flag_get("boss_mine")){ set_combat_return(SceneId::WORLD); start_combat(9,"/sdcard/ASTERIA/battle/CHEF.BMP"); mgr.set(SceneId::COMBAT); return; }
          if(!_tp && _loc==20 && py<=20 && !flag_get("boss_corbeau")){ if(quest_status(11)==0) quest_start(11);
            notice=true; noticeTxt="Corbeau Noir, le chef des brigands, te barre la route !"; flag_set("vu_corbeau");
            set_combat_return(SceneId::WORLD); start_combat(25,backdrop(20)); mgr.set(SceneId::COMBAT); return; }
          int cape = item_index("OBJ_0024");
          int mod = (cape>=0 && is_equipped(player(),cape)) ? 12 : 9;   // cape portee
          if(daytime::is_night()) mod = mod*2/3;                       // la nuit, les rencontres sont plus frequentes
          else if(_loc==8 || _loc==20) mod = mod*4/3;                  // de jour, foret et route sont plus calmes (1 pas sur 12, 16 avec la cape)
          if(!_tp && !g_no_encounters && (_loc==8||_loc==9||_loc==20) && (ws_rand()%mod==0)){
            int _m=(_loc==8)?5+(int)(ws_rand()%4):(_loc==20)?(10+(int)(ws_rand()%2)):((ws_rand()&1u)?16:23);
            set_combat_return(SceneId::WORLD); start_combat(_m, backdrop(_loc));
            mgr.set(SceneId::COMBAT); return;
          }
        }
      }
    }
  }

  if(p&gb::BTN_A){ int fx=px,fy=py; if(dir==0)fy++; else if(dir==1)fy--; else if(dir==2)fx--; else fx++;
    int n=npc_at(M,fx,fy);
    int _loc=MAPS[map].location;
    if(_loc==1 && n==13){ mgr.set(SceneId::INN); }
    else if(_loc==1 && n==10){ mgr.set(SceneId::DICE); }
    else if(n==2){ mgr.set(SceneId::SHOP); }
    else if(n>=0 && n<22 && dlg::DIALOG[n].nnodes>0){
      // --- Annexes de l'Acte I (démarrage au 1er dialogue, remise au retour) ---
      if(n==1){   // Doran : Q1 Les Outils perdus (remise = rapporter le casque de la mine), puis Forge & Alchimie
        if(flag_get("doran_resolu")){ mgr.set(SceneId::FORGE); return; }   // visites suivantes : ouvre la Forge & Alchimie
        if(quest_status(1)==0) quest_start(1);
        else if(quest_status(1)==1 && player().inv.has(item_index("OBJ_0010"),1)){ complete_quest_reward(1); flag_set("doran_resolu"); }   // remise : resout Q1 (forge dispo des la prochaine visite)
      } else if(n==8){
        if(quest_status(4)==0){ quest_start(4); notice=true; noticeTxt="Agnès : rapporte-moi 5 herbes médicinales et 3 champignons de la Forêt de Brume."; }
        else if(quest_status(4)==1){ int h=item_index("OBJ_0034"), c=item_index("OBJ_0035");
          if(player().inv.has(h,5) && player().inv.has(c,3)){ player().inv.remove(h,5); player().inv.remove(c,3); complete_quest_reward(4); }
          else { notice=true; noticeTxt="Agnès : il me faut 5 herbes médicinales et 3 champignons."; } }
      } else if(n==5){
        if(quest_status(3)==1) complete_quest_reward(3);
      } else if(n==13){
        int pn=item_index("OBJ_0014");
        if(pn>=0 && player().inv.has(pn,1)){
          if(quest_status(7)==0) quest_start(7);   // 1re présentation : ouvre l'enquête
          else if(quest_status(7)==1){ complete_quest_reward(7);   // retour : consultation terminée
            if(quest_status(13)==0) quest_start(13); }   // Albin oriente vers les Pierres Murmurantes (Q13)
        }
      } else if(n==15 && _loc==10){
        if(!flag_get("q8_pris")){ flag_set("q8_pris"); if(quest_status(8)==0) quest_start(8);
          int pli=item_index("OBJ_0039"); if(pli>=0 && !player().inv.has(pli,1) && player().inv.add(pli,1)) item_ui::pickup_push(pli,1); }
        else if(!flag_get("q8_fait") && flag_get("c_pli") && flag_get("c_herbes") && flag_get("c_aumone")){
          flag_set("q8_fait"); complete_quest_reward(8); }
      } else if(n==16 && _loc==10 && flag_get("q8_pris") && !flag_get("c_pli")){
        int pli=item_index("OBJ_0039"); if(pli>=0 && player().inv.has(pli,1)){ player().inv.remove(pli,1); flag_set("c_pli"); }
      } else if(n==16 && _loc==20){
        if(quest_status(12)==0){ quest_start(12); flag_set("q12_pris"); }
        else if(quest_status(12)==1 && flag_get("boss_corbeau") && !flag_get("q12_fait")){ flag_set("q12_fait"); player().gold+=40; complete_quest_reward(12); }
      } else if(n==17 && _loc==10 && flag_get("q8_pris") && !flag_get("c_herbes")){
        int h=item_index("OBJ_0034"); if(h>=0 && player().inv.has(h,3)){ player().inv.remove(h,3); flag_set("c_herbes"); }
      } else if(n==12 && _loc==10 && flag_get("q8_pris") && !flag_get("c_aumone")){
        int pot=item_index("OBJ_0005"); if(pot>=0 && player().inv.has(pot,1)){ player().inv.remove(pot,1); flag_set("c_aumone"); }
      } else if(n==19 && _loc==10){
        if(flag_get("vu_meluve")){ int col=item_index("OBJ_0038");   // 1re rencontre : noeud "!vu_meluve" (drapeau + Q15 en fin de dialogue)
          if(col>=0 && player().inv.has(col,1) && !flag_get("q15_fait")){ flag_set("q15_fait"); player().gold+=30;   /* Meluve le laisse : « Garde-le » (collier equipable, +2 AU) */ complete_quest_reward(15); } }
      } else if(n==20 && !flag_get("anneau_recu")){
        flag_set("anneau_recu"); int rg=item_index("OBJ_0040"); if(rg>=0 && !player().inv.has(rg,1) && player().inv.add(rg,1)) item_ui::pickup_push(rg,1);
        complete_quest_reward(16);
      } else if(n==20 && !flag_get("orwenn_apres")) flag_set("orwenn_apres");
      else if(n==21 && !flag_get("vu_voyageur")){   // Voyageur Encapuchonne : remet le Livre des Elements (Arc 0)
        flag_set("vu_voyageur");
        int lv=item_index("OBJ_0041"); if(lv>=0 && !player().inv.has(lv,1) && player().inv.add(lv,1)) item_ui::pickup_push(lv,1);
      }
      dlg=true; dlgNpc=n; dlgNode=pick_node(dlg::DIALOG[n]); dlgLine=0;
    } else if(MAPS[map].location==23 && ((fx==5&&fy==9)||(fx==12&&fy==4)||(fx==20&&fy==7)||(fx==21&&fy==16))){
      static const char* PM[]={"pm6","pm12","pm22","pm36"}; const char* mf=(fx==5)?PM[0]:(fx==12)?PM[1]:(fx==20)?PM[2]:PM[3];
      if(quest_status(13)<1){ notice=true; noticeTxt="Un monolithe gravé, muet. Maître Albin saurait peut-être le lire."; }
      else if(!player().inv.has(item_index("OBJ_0014"),1)){ notice=true; noticeTxt="La pierre reste muette. Il te faudrait la Pierre noire gravée."; }
      else if(flag_get(mf)){ notice=true; noticeTxt="Cette pierre murmurante est déjà éveillée."; }
      else { flag_set(mf); notice=true; noticeTxt="La Pierre noire effleure le monolithe : il s'illumine et murmure.";
        if(flag_get("pm6")&&flag_get("pm12")&&flag_get("pm22")&&flag_get("pm36")){
          if(quest_status(13)==1) complete_quest_reward(13);
          noticeTxt="Les quatre pierres résonnent à l'unisson. Le chemin du Temple t'est révélé."; } }
    } else {
      int ci=chest_at(map,fx,fy);
      if(ci>=0){ const ChestDef& c=CHESTS[ci];
        if(c.item>=0 && !player().inv.add(c.item,1)){   // BUG-07 : inventaire plein -> on laisse le coffre FERME, le joueur reviendra
          notice=true; noticeTxt="Ton inventaire est plein ! Fais de la place et reviens ouvrir ce coffre."; audio::sfx_error();
        } else {
          player().gold+=c.gold; flag_set(c.flag); asteria::audio::sfx_chest();
          if(c.item>=0) item_ui::pickup_push(c.item,1);           // fenetres "Objet obtenu" : l'objet puis l'or
          if(c.gold>0)  item_ui::pickup_push(item_ui::GOLD,c.gold);
          if(c.item<0 && c.gold<=0){ notice=true; noticeTxt="Un coffre ! Tu récupères du butin."; }
          if(c.complete_q>=0){
            if(quest_status(c.complete_q)==0) quest_start(c.complete_q);
            if(quest_status(c.complete_q)==1) complete_quest_reward(c.complete_q);
            if(c.setflag) flag_set(c.setflag);
            if(c.complete_q==5||c.complete_q==10) notice=true;     // note de quete trouvee dans le coffre
            if(c.complete_q==5) noticeTxt="Un tissu déchiré, arraché à un villageois... Les traces mènent à la Vieille Mine.";
            else if(c.complete_q==10) noticeTxt="Le carnet du chef de caravane ! Ses notes mènent à Grand-Castel.";
          }
        }
      } else if(fx>=0&&fy>=0&&fx<M.w&&fy<M.h && M.tiles[fy*M.w+fx]==4 && !door_leads_somewhere(M,fx,fy)){
        notice=true; noticeTxt=i18n::tr(i18n::W_CLOSED);
      }
    }
  }

  if(p&gb::BTN_RUN){ charwin::open(0); return; }   // raccourci : fenetre personnage
  if(p&(gb::BTN_MENU|gb::BTN_B)){ confirmQuit=true; pauseSel=0; pauseSub=0; pauseListSel=0; pauseMsg=nullptr; }
}

// ---------------------------------------------------------------------------
// Dessin d'UNE tuile de carte en coordonnees CARTE (pixels absolus x*16,y*16).
// L'appelant fixe gb::set_origin(camx,camy) pour la placer a l'ecran, ou dessine
// dans le cache de carte (origine 0). Les variations pseudo-aleatoires des tuiles
// (hash des arbres, rives...) dependent donc de la position sur la CARTE et ne
// changent plus quand la camera defile.
// ---------------------------------------------------------------------------
void draw_map_tile(const Map& M,int x,int y){
  uint8_t _t=M.tiles[y*M.w+x]; int dx=x*TILE_SIZE, dy=y*TILE_SIZE;
      if(_t==3){ auto W=[&](int xx,int yy){ if(xx<0||yy<0||xx>=M.w||yy>=M.h) return true; int t=M.tiles[yy*M.w+xx]; return t==3||t==5; };   // hors-carte = eau continue -> pas de rive au bord, l'eau s'ecoule
        draw_natural_water(dx,dy, W(x,y-1),W(x,y+1),W(x+1,y),W(x-1,y), W(x+1,y-1),W(x-1,y-1),W(x+1,y+1),W(x-1,y+1)); }
      else if(_t==1){ auto P=[&](int xx,int yy){ if(xx<0||yy<0||xx>=M.w||yy>=M.h) return true; return M.tiles[yy*M.w+xx]==1; };   // hors-carte = chemin continue -> il sort de la carte au lieu de s'arreter
        draw_natural_path(dx,dy, P(x,y-1),P(x,y+1),P(x+1,y),P(x-1,y), P(x+1,y-1),P(x-1,y-1),P(x+1,y+1),P(x-1,y+1)); }
      else if(_t==16){ auto N=[&](int xx,int yy){ if(xx<0||yy<0||xx>=M.w||yy>=M.h) return true; int t=M.tiles[yy*M.w+xx]; return t==16||t==5; };   // hors-carte + pont = eau -> pas de rive au bord ni au pont
        draw_natural_water(dx,dy, N(x,y-1),N(x,y+1),N(x+1,y),N(x-1,y), N(x+1,y-1),N(x-1,y-1),N(x+1,y+1),N(x-1,y+1)); }
      else if(_t==19){ auto R=[&](int xx,int yy){ if(xx<0||yy<0||xx>=M.w||yy>=M.h) return true; return M.tiles[yy*M.w+xx]==19; };   // hors-carte = route continue
        draw_natural_road(dx,dy, R(x,y-1),R(x,y+1),R(x+1,y),R(x-1,y), R(x+1,y-1),R(x-1,y-1),R(x+1,y+1),R(x-1,y+1)); }
      else if(_t==7||_t==9||_t==10||_t==11){ auto R=[&](int xx,int yy){ if(xx<0||yy<0||xx>=M.w||yy>=M.h) return false; uint8_t q=M.tiles[yy*M.w+xx]; return q==7||q==9||q==10||q==11; };
        draw_roof_tile(dx,dy, R(x,y-1),R(x,y+1),R(x-1,y),R(x+1,y), _t); }
      else if(_t==2||_t==13||_t==14||_t==15 || ((_t==4||_t==8||_t==12) && ((x>0&&is_facade_tile(M.tiles[y*M.w+x-1]))||(x+1<M.w&&is_facade_tile(M.tiles[y*M.w+x+1]))))){
        auto F=[&](int xx,int yy){ if(xx<0||yy<0||xx>=M.w||yy>=M.h) return false; return is_facade_tile(M.tiles[yy*M.w+xx]); };
        int mat=facade_material(M.tiles,M.w,M.h,x,y);
        draw_facade_tile(dx,dy, F(x,y-1),F(x,y+1),F(x-1,y),F(x+1,y), _t, mat); }
      else if(_t==21){ auto Rm=[&](int xx,int yy){ if(xx<0||yy<0||xx>=M.w||yy>=M.h) return false; return M.tiles[yy*M.w+xx]==21; };
        draw_rampart_tile(dx,dy, Rm(x,y-1),Rm(x,y+1),Rm(x-1,y),Rm(x+1,y)); }
      else if(_t==6) draw_tree(dx,dy);
      else if(_t==22) draw_tower(dx,dy);
      else if(_t==24){ auto Tb=[&](int xx,int yy){ if(xx<0||yy<0||xx>=M.w||yy>=M.h) return false; return M.tiles[yy*M.w+xx]==24; };
        draw_table_tile(dx,dy, Tb(x,y-1),Tb(x,y+1),Tb(x-1,y),Tb(x+1,y)); }
      else if(_t>=23&&_t<=27) draw_inn_tile(dx,dy,_t);
      else if(_t==30) draw_hearth(dx,dy);
      else if(_t==32) draw_round_table(dx,dy);
      else if(_t==31){ auto G=[&](int xx,int yy){ if(xx<0||yy<0||xx>=M.w||yy>=M.h) return false; return M.tiles[yy*M.w+xx]==31; };
        draw_rug(dx,dy, G(x,y-1),G(x,y+1),G(x-1,y),G(x+1,y)); }
      else if(_t==0) draw_grass(dx,dy,x,y);
      else if(_t==5){ auto Wt=[&](int xx,int yy){ if(xx<0||yy<0||xx>=M.w||yy>=M.h) return false; int t=M.tiles[yy*M.w+xx]; return t==16||t==3; };   // cotes eau -> garde-corps + planches
        draw_bridge(dx,dy, Wt(x-1,y),Wt(x+1,y),Wt(x,y-1),Wt(x,y+1)); }
      else if(_t==28) gb::draw_image(spr::MONT_SPR.px,16,16,dx,dy);   // rocher/mont (importe Wargame)
      else if(_t==29) draw_grove(dx,dy);   // bosquet : style maison (uniforme)
      else if(_t<23) gb::draw_image(spr::TILE[_t].px,spr::TILE[_t].w,spr::TILE[_t].h,dx,dy);
      else gb::fill_rect(dx,dy,TILE_SIZE,TILE_SIZE,TCOL(_t));
}

// ---------------------------------------------------------------------------
// OPT-AKA-3 : cache de la carte entiere (rendu procedural fait UNE fois par carte).
// Le decor est statique : on le rend dans un tampon (PSRAM sur AKA, ~650 Ko pour la
// plus grande carte) a l'arrivee sur la carte, puis chaque image ne fait qu'un blit
// de la fenetre visible + le redessin, sous chaque personnage, des tuiles de la
// rangee suivante (arbres, toits) qui doivent le masquer. Rendu identique au dessin
// direct. Retour au dessin tuile par tuile : -DASTERIA_MAP_CACHE=0 (ou si
// l'allocation echoue).
// ---------------------------------------------------------------------------
#ifndef ASTERIA_MAP_CACHE
#define ASTERIA_MAP_CACHE 1
#endif
static uint16_t* s_mc=nullptr; static int s_mcMap=-1, s_mcW=0, s_mcH=0; static unsigned s_mcCap=0;
static bool map_cache_ready(int map){
#if ASTERIA_MAP_CACHE
  if(s_mc && s_mcMap==map) return true;
  const Map& M=MAPS[map]; int W=M.w*TILE_SIZE, H=M.h*TILE_SIZE; unsigned need=(unsigned)W*(unsigned)H*2u;
  if(need>s_mcCap){ if(s_mc) gb::free_big(s_mc); s_mc=(uint16_t*)gb::alloc_big(need); s_mcCap=s_mc?need:0; }
  if(!s_mc){ s_mcMap=-1; return false; }
  s_mcW=W; s_mcH=H; s_mcMap=map;
  gb::reset_raster(); gb::set_target(s_mc,W,H);
  for(int i=0;i<W*H;i++) s_mc[i]=0;
  for(int y=0;y<M.h;++y) for(int x=0;x<M.w;++x) draw_map_tile(M,x,y);
  gb::reset_raster();
  return true;
#else
  (void)map; return false;
#endif
}

// Redessine, limitees au rectangle ecran (bx,by,bw,bh), les tuiles des rangees > row
// qui recouvrent ce rectangle (les arbres debordent de 14 px vers le haut, 3 px
// sur les cotes) : reproduit l'ordre de dessin d'origine par-dessus un personnage.
static void occlude(const Map& M,int row,int camx,int camy,int bx,int by,int bw,int bh){
  int mx0=(bx+camx-19)/TILE_SIZE, mx1=(bx+bw+camx+3)/TILE_SIZE;
  if(mx0<0)mx0=0; if(mx1>=M.w)mx1=M.w-1;
  gb::set_origin(camx,camy); gb::set_clip(bx,by,bw,bh);
  for(int y=row+1;y<=row+2 && y<M.h;++y) for(int x=mx0;x<=mx1;++x) draw_map_tile(M,x,y);
  gb::reset_raster();
}

void WorldScene::render(){
  const Map& M=MAPS[map];
  // Camera a zone morte, sur les 2 axes. La zone de jeu visible va de x=0 a 320 et
  // de y=HUD_H (sous le bandeau) a 240. La camera ne bouge que lorsque le heros sort
  // du tiers central de cette zone (haut/bas/gauche/droite), puis s'arrete sur les
  // bords de la carte. En haut elle descend jusqu'a camy=-HUD_H : la rangee 0
  // s'affiche juste sous le bandeau au lieu d'etre masquee (rangees 0-1 accessibles).
  const int HUD_H=32, VIEW_W=320, VIEW_H=240-HUD_H, ZX=VIEW_W/3, ZY=VIEW_H/3;
  if(camMap!=map){ camMap=map;                                   // arrivee sur la carte : heros au centre de la zone
    camX=px*TILE_SIZE+TILE_SIZE/2-VIEW_W/2; camY=py*TILE_SIZE+TILE_SIZE/2-(HUD_H+VIEW_H/2); }
  { int sx=px*TILE_SIZE-camX, sy=py*TILE_SIZE-camY;              // coin haut-gauche de la tuile du heros, a l'ecran
    if(sx<ZX)                           camX=px*TILE_SIZE-ZX;                          // gauche du 1er tiers
    else if(sx+TILE_SIZE>VIEW_W-ZX)     camX=px*TILE_SIZE+TILE_SIZE-(VIEW_W-ZX);       // droite du 3e tiers
    if(sy<HUD_H+ZY)                     camY=py*TILE_SIZE-(HUD_H+ZY);                  // au-dessus du 1er tiers
    else if(sy+TILE_SIZE>240-ZY)        camY=py*TILE_SIZE+TILE_SIZE-(240-ZY); }        // sous le 3e tiers
  int maxx=M.w*TILE_SIZE-VIEW_W, miny=-HUD_H, maxy=M.h*TILE_SIZE-240;
  if(maxx<0) camX=maxx/2;                                         // carte moins large que l'ecran : centree
  else { if(camX<0)camX=0; if(camX>maxx)camX=maxx; }
  if(maxy<miny) camY=miny-(VIEW_H-M.h*TILE_SIZE)/2;             // carte moins haute que la zone : centree sous le bandeau
  else { if(camY<miny)camY=miny; if(camY>maxy)camY=maxy; }
  int camx=camX, camy=camY;
  int x0=camx/TILE_SIZE-1, y0=camy/TILE_SIZE-1, x1=x0+25, y1=y0+20;
  if(x0<0)x0=0; if(y0<0)y0=0; if(x1>M.w)x1=M.w; if(y1>M.h)y1=M.h;

  const bool cached=map_cache_ready(map);
  if(cached){   // fenetre visible du cache (cartes plus petites que l'ecran : centrees)
    int sx=camx, sy=camy, dx=0, dy=0, w=320, h=240;
    if(sx<0){ dx=-sx; w+=sx; sx=0; } if(sy<0){ dy=-sy; h+=sy; sy=0; }
    if(sx+w>s_mcW) w=s_mcW-sx; if(sy+h>s_mcH) h=s_mcH-sy;
    if(dx>0||dy>0||w<320||h<240) gb::clear(gb::rgb(0,0,0));
    if(w>0&&h>0) gb::blit_region(s_mc,s_mcW,s_mcH,sx,sy,w,h,dx,dy);
  }
  for(int y=y0;y<y1;++y){
    if(!cached){ gb::set_origin(camx,camy); for(int x=x0;x<x1;++x) draw_map_tile(M,x,y); gb::reset_raster(); }
    // Tri par profondeur : les entites de la rangee y sont dessinees APRES ses tuiles.
    // Un arbre/tour d'une rangee inferieure (en avant) sera dessine ensuite -> il masque le heros, qui "passe derriere".
    for(int i=0;i<M.spawn_n;++i){ const Spawn& s=M.spawns[i]; if(s.y!=y) continue;
      if(s.npc>=0 && s.npc<spr::NPC_COUNT && spr::NPC[s.npc].px){
        const spr::Sprite& ns=spr::NPC[s.npc];
        int nx=s.x*TILE_SIZE+(TILE_SIZE-ns.w)/2-camx, ny=s.y*TILE_SIZE+TILE_SIZE-ns.h+2-camy;
        gb::draw_image_key(ns.px,ns.w,ns.h,nx,ny,spr::KEY);
        if(cached) occlude(M,y,camx,camy,nx,ny,ns.w,ns.h);
      } else gb::fill_rect(s.x*TILE_SIZE+4-camx,s.y*TILE_SIZE+2-camy,8,12,gb::rgb(230,220,120)); }
    if(py==y){
      int fr = (gb::millis()-lastMove < 300) ? (int)((gb::millis()/120)%3) : 1;
      const spr::Sprite& hs=spr::HERO[dir][fr];
      int sx=px*TILE_SIZE+(TILE_SIZE-hs.w)/2-camx; int sy=py*TILE_SIZE+TILE_SIZE-hs.h+2-camy;
      gb::draw_image_key(hs.px,hs.w,hs.h,sx,sy,spr::KEY);
      if(cached) occlude(M,y,camx,camy,sx,sy,hs.w,hs.h);   // arbres/toits de la rangee suivante par-dessus le heros
      // marqueur : petite fleche doree au-dessus du heros quand il est entoure d'arbres/objets hauts
      auto tall=[&](int xx,int yy){ if(xx<0||yy<0||xx>=M.w||yy>=M.h) return false; uint8_t q=M.tiles[yy*M.w+xx]; return q==6||q==22||q==28||q==29||q==2||q==13||q==14||q==15; };
      if(tall(px,py-1)||tall(px,py+1)||tall(px-1,py)||tall(px+1,py)){
        int cxm=px*TILE_SIZE+TILE_SIZE/2-camx, ym=sy-7;
        gb::fill_rect(cxm-4,ym-1,9,1,gb::rgb(20,16,10));
        for(int r=0;r<4;r++) gb::fill_rect(cxm-(3-r),ym+r,(3-r)*2+1,1,gb::rgb(245,215,95)); }
    }
  }
  if(py<y0||py>=y1){
    int fr = (gb::millis()-lastMove < 300) ? (int)((gb::millis()/120)%3) : 1;
    const spr::Sprite& hs=spr::HERO[dir][fr];
    int sx=px*TILE_SIZE+(TILE_SIZE-hs.w)/2-camx; int sy=py*TILE_SIZE+TILE_SIZE-hs.h+2-camy;
    gb::draw_image_key(hs.px,hs.w,hs.h,sx,sy,spr::KEY);
  }

  for(int ci=0;ci<NCHEST;ci++){
    if(chest_at(map,CHESTS[ci].x,CHESTS[ci].y)==ci){
      int cx=CHESTS[ci].x*TILE_SIZE-camx, cy=CHESTS[ci].y*TILE_SIZE-camy;
      if(cx<-16||cy<-16||cx>320||cy>240) continue;   // hors ecran -> on saute
      draw_chest_closed(cx,cy);
    }
  }

  // --- Jour / nuit : eclairage de la zone de jeu (sous le bandeau), seulement dehors ---
  if(is_outdoor(M.location)){
    int amb=daytime::ambient();
    if(amb<256){
      gb::Light L[gb::LIGHT_MAX]; int n=0;
      int dark=256-amb;                                              // 0 (jour) .. ~164 (nuit noire)
      L[n++]={ (int16_t)(px*TILE_SIZE+8-camx), (int16_t)(py*TILE_SIZE+6-camy), (int16_t)(34+dark/5), 70 };   // halo du heros
      if(dark>60){                                                   // fenetres / portes allumees a la tombee du jour
        for(int y=y0;y<y1 && n<gb::LIGHT_MAX;++y) for(int x=x0;x<x1 && n<gb::LIGHT_MAX;++x)
          if(M.tiles[y*M.w+x]==4) L[n++]={ (int16_t)(x*TILE_SIZE+8-camx), (int16_t)(y*TILE_SIZE+10-camy), (int16_t)(22+dark/10), 200 };
      }
      gb::light_screen(32,240,amb,L,n);
    }
  }
  if(dlg && choice::active()) choice::render(dlg::DIALOG[dlgNpc].name);   // choix d'attitude
  else if(dlg){ const dlg::NpcDialog& D=dlg::DIALOG[dlgNpc];
    gb::fill_rect(8,158,304,74,gb::rgb(20,16,10));
    gb::fill_rect(8,158,304,2,gb::rgb(200,170,90)); gb::fill_rect(8,230,304,2,gb::rgb(200,170,90));
    ui::gold(16,162,trc(D.name),true);
    const dlg::Node& _N=D.nodes[dlgNode]; const char* t=(dlgLine<_N.nlines)?hero_text(trc(_N.lines[dlgLine])):"";
    // wrap simple ~38 c
    char l1[64]={0},l2[64]={0}; int n=0; while(t[n])n++;
    if(n<=38){ for(int i=0;i<n;i++)l1[i]=t[i]; }
    else { int cut=38; while(cut>0&&t[cut]!=' ')cut--; if(cut==0)cut=38;
      for(int i=0;i<cut;i++)l1[i]=t[i]; int k=0; for(int i=cut+1;t[i]&&k<63;i++)l2[k++]=t[i]; }   // SEC-03 : borne k<63
    ui::text(16,182,l1,gb::rgb(236,226,192)); if(l2[0]) ui::text(16,198,l2,gb::rgb(236,226,192));
    ui::text(192,214,i18n::tr(i18n::W_DLG_H),gb::rgb(170,160,132));
  }

  // --- Bandeau HUD (2 lignes reservees en haut) : lieu + quete, sans aucun chevauchement ---
  { gb::fill_rect(0,0,320,30,gb::rgb(20,16,10));
    gb::fill_rect(0,30,320,2,gb::rgb(200,170,90));
    int qi=hud_quest();
    if(qi<0) for(int i=0;i<quest::QUEST_COUNT;i++){ if(quest_status(i)==2){qi=i;break;} }
    // Ligne 1 : nom clair de la zone (gauche, traduit) + nom de la quete (droite si la place le permet)
    const char* lraw=loc_name(MAPS[map].location); const char* ln=lraw[0]?trc(lraw):"";
    if(ln[0]) ui::gold(6,3,ln,false);
    // Horloge (droite) : soleil ou lune + heure ; le nom de la quete se place a sa gauche.
    int clockX=314;
    {
      char hh[16]; int m=daytime::minutes(); snprintf(hh,sizeof(hh),"%dh%02d",(m/60)%24,m%60);
      int tw=ui::text_w(hh); clockX=314-tw-14;
      if(daytime::is_night()){ for(int dy=-4;dy<=4;dy++) for(int dx=-4;dx<=4;dx++){ int d=dx*dx+dy*dy, e=(dx-2)*(dx-2)+(dy+1)*(dy+1);
          if(d<=16 && e>9) gb::pixel(clockX+6+dx,9+dy,gb::rgb(220,225,255)); } }          // croissant de lune
      else { gb::fill_rect(clockX+3,6,7,7,gb::rgb(250,205,70)); gb::fill_rect(clockX+5,4,3,11,gb::rgb(250,205,70)); gb::fill_rect(clockX+1,8,11,3,gb::rgb(250,205,70)); }   // soleil
      ui::text(clockX+14,4,hh,gb::rgb(230,220,180));
    }
    if(qi>=0){ const char* qn=trc(quest::QUEST[qi].name); int qx=clockX-6-ui::text_w(qn);
      if(qx > 6+ui::text_w(ln)+14) ui::text(qx,4,qn,gb::rgb(230,220,180)); }
    // Ligne 2 : objectif courant (gauche)
    if(qi>=0){ int qs=quest_status(qi);
      if(qs==1){ // prochain objectif reel (deduit de l'etat du jeu, comme dans le journal)
        ui::gold(6,16,">",true); ui::text(16,16,trc(journal::next_objective(qi,nullptr)),gb::rgb(220,210,180)); }
      else if(qs==2){ ui::gold(6,16,i18n::tr(i18n::W_QDONE),true); } }
  }

  if(notice){ gb::fill_rect(8,158,304,74,gb::rgb(20,16,10)); gb::fill_rect(8,158,304,2,gb::rgb(200,170,90)); gb::fill_rect(8,230,304,2,gb::rgb(200,170,90));
    ui::gold(16,162,i18n::tr(i18n::W_EVENT),true);
    // wrap ~38
    const char* t=noticeTxt?hero_text(trc(noticeTxt)):""; char a[64]={0},b[64]={0}; int n=0; while(t[n])n++;
    if(n<=38){ for(int i=0;i<n;i++)a[i]=t[i]; } else { int cut=38; while(cut>0&&t[cut]!=' ')cut--; if(!cut)cut=38; for(int i=0;i<cut;i++)a[i]=t[i]; int k=0; for(int i=cut+1;t[i]&&k<63;i++)b[k++]=t[i]; }   // SEC-03 : borne k<63
    ui::text(16,182,a,gb::rgb(236,226,192)); if(b[0])ui::text(16,198,b,gb::rgb(236,226,192));
    ui::text(206,214,i18n::tr(i18n::W_CONT),gb::rgb(170,160,132));
  }

  if(!notice && !dlg && !confirmQuit) item_ui::pickup_render();   // fenetre "Objet obtenu"

  if(confirmQuit){   // DELTA-03 : menu pause
    gb::fill_rect(14,34,292,196,gb::rgb(20,16,10));
    gb::fill_rect(14,34,292,2,gb::rgb(200,170,90));
    gb::fill_rect(14,228,292,2,gb::rgb(200,170,90));

    char st[40]; int pv=player().VIT-player().BL; if(pv<0)pv=0;
    snprintf(st,sizeof(st),"PV %d/%d  PM %d/%d",pv,player().VIT,player().mana,player().manaMax);
    if(pauseSub==0) ui::text(154,40,st,gb::rgb(200,190,160));   // sous-menus : titre long, pas de chevauchement

    if(pauseSub==0){
      ui::gold(24,40,trc("Pause"),true);
      char saveLbl[32]; snprintf(saveLbl,sizeof(saveLbl),trc("Sauvegarder (Empl.%d)"),active_save_slot());
      const char* MENU[7]={trc("Reprendre"),trc("Personnage"),trc("Journal de quêtes"),trc("Carte du monde"),trc("Alchimie & Soins"),saveLbl,trc("Quitter vers le titre")};
      for(int i=0;i<7;i++){ int y=62+i*20;
        if(i==pauseSel){ gb::fill_rect(22,y-2,276,17,gb::rgb(60,45,20)); ui::gold(26,y,">",true); }
        ui::text(40,y,MENU[i],gb::rgb(235,225,190)); }
      ui::text(24,208,trc("A: ouvrir   B: reprendre"),gb::rgb(170,160,132));
    } else if(pauseSub==2){
      ui::gold(24,40,trc("Alchimie & Soins"),true);
      int kinds[8], ids[8]; int n=alch_entries(kinds,ids);
      for(int i=0;i<n && i<6;i++){ int y=62+i*19;
        if(i==pauseListSel){ gb::fill_rect(22,y-2,276,16,gb::rgb(60,45,20)); ui::gold(24,y,">",true); }
        char ln[56];
        if(kinds[i]==0||kinds[i]==1) snprintf(ln,sizeof(ln),trc("Sort : %s (%d PM)"),trc(ids[i]==4?"Soins":"Guérison maj."),spell_cost(ids[i]));
        else { const Recipe& r=RECIPES[ids[i]]; snprintf(ln,sizeof(ln),trc("Alchimie : %s"),trc(ITEMS[r.out_item].name)); }
        ui::text(36,y,ln,gb::rgb(235,225,190));
      }
      if(pauseMsg) ui::text(24,184,trc(pauseMsg),gb::rgb(245,215,95));
      else if(n>0 && kinds[pauseListSel]==2){
        const Recipe& r=RECIPES[ids[pauseListSel]];
        char ing[64]={0}; int o=0;
        for(int k=0;k<r.input_n && o<50;k++) o+=snprintf(ing+o,sizeof(ing)-o,"%s%dx %s",k?" + ":"",r.inputs[k].qty,trc(ITEMS[r.inputs[k].item].name));
        ui::text(24,184,ing,gb::rgb(185,175,145));
      }
      ui::text(24,208,trc("A: lancer / préparer   B: retour"),gb::rgb(170,160,132));
    } else if(pauseSub==4){
      ui::gold(24,40,trc("Sauvegarde (3 empl.)"),true);
      for(int s=1;s<=3;s++){
        const SaveSummary& sm=g_slotSum[s-1];
        int y=68+(s-1)*28;
        if((s-1)==pauseListSel){ gb::fill_rect(22,y-3,276,22,gb::rgb(60,45,20)); ui::gold(26,y,">",true); }
        char ln[52];
        if(sm.exists) snprintf(ln,sizeof(ln),trc("Empl.%d : %s (Niv.%d, %d or)"),s,sm.name,sm.level,sm.gold);
        else snprintf(ln,sizeof(ln),trc("Empl.%d : [Emplacement vide]"),s);
        ui::text(40,y,ln,gb::rgb(235,225,190));
      }
      if(pauseMsg) ui::text(24,180,trc(pauseMsg),gb::rgb(185,235,175));
      ui::text(24,208,trc("A: sauvegarder ici   B: retour"),gb::rgb(170,160,132));
    }
  }
  journal::render();
  worldmap::render();
  charwin::render();   // fenetre personnage (au-dessus du menu pause si ouverte depuis celui-ci)
}
}
