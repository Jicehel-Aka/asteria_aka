// Validation des quetes secondaires : teleporte le joueur au donneur et dialogue,
// en accordant les prerequis (objets) -> teste le CABLAGE (pas la cueillette).
#include "scene/scene.h"
#include "scene/world_scene.h"
#include "platform/gb_port.h"
#include "player.h"
#include "gamestate.h"
#include "progress.h"
#include "audio/jingle.h"
#include <cstdio>
void host_set_budget(int n); void host_inject(int code); void host_tick();
using namespace asteria;
static SceneManager* MG;
static void step(int code){ host_inject(code); MG->update(); host_inject(-1); host_tick(); }
// face la cible puis A, puis avance le dialogue/notice jusqu'au bout
static void talk(int px,int py,int faceCode){
  WorldScene& w=world_scene(); w.px=px; w.py=py; w.notice=false; w.dlg=false;
  step(faceCode);                 // orienter
  step(5);                        // A : ouvre le dialogue (+ annexe de quete)
  for(int i=0;i<16 && (w.dlg||w.notice);i++) step(5);  // avancer / fermer
}
static void grant(const char* id,int n){ int k=item_index(id); if(k>=0) for(int i=0;i<n;i++) player().inv.add(k,1); }
static int Q(int n){ return quest_status(n); }

int main(){
  audio::init(); host_set_budget(1000000000);
  new_game_reset();
  SceneManager mgr; MG=&mgr; mgr.set(SceneId::WORLD);
  printf("== VALIDATION DES QUETES SECONDAIRES ==\n");

  // --- Valbois (loc0) ---
  world_scene().load_map(0); step(-1); // declenche valbois_intro + quest_start(0)
  // Q0 : maire npc0 (6,8) -> parler
  talk(6,9,1);
  printf("Q0 (maire)            : %d\n",Q(0));
  // Q16 : Dame Orwenn npc20 (14,20)
  talk(14,21,1);
  printf("Q16 (anneau Orwenn)   : %d  anneau=%d\n",Q(16),player().inv.has(item_index("OBJ_0040"),1));
  // Q4 : Agnes npc8 (18,12) -> 2 visites
  talk(18,13,1); talk(18,13,1);
  printf("Q4 (plantes Agnes)    : %d\n",Q(4));
  // Q1 : Doran npc1 (14,12) -> besoin du casque OBJ_0010 ; 1re visite lance, 2e termine
  talk(14,13,1);                      // lance Q1
  grant("OBJ_0010",1);                // le joueur a recupere le casque (boss mine)
  talk(14,13,1);                      // remise
  printf("Q1 (outils Doran)     : %d\n",Q(1));
  // Q2 auto-start quand Q0==2 (fait a la prochaine update)
  step(-1); step(-1);
  printf("Q2 demarree ? status  : %d (attendu 1)\n",Q(2));

  // --- Foret (loc8) : Lysa npc5 (11,18) : 1er dialogue termine Q2 + lance Q5, Q3 auto, 2e termine Q3
  world_scene().load_map(1); step(-1);
  talk(11,17,2);   // stand (12,18) face LEFT vers (11,18)? on se place a droite
  // en fait placons-nous a (12,18) et regardons a gauche (code 3)
  talk(12,18,3);
  step(-1); step(-1);   // Q3 auto-start si Q2==2
  talk(12,18,3);        // 2e visite -> termine Q3
  printf("Q2 (traces)           : %d\n",Q(2));
  printf("Q3 (empreintes)       : %d\n",Q(3));
  printf("Q5 (tissu) demarree   : %d\n",Q(5));
  // Cueillette des herbes en FORET (pour Q8) : marcher sur chaque plante
  { WorldScene& w=world_scene(); int before=player().inv.has(item_index("OBJ_0034"),1);
    int hx[]={17,20,19}, hy[]={2,8,12};
    for(int i=0;i<3;i++){ w.px=hx[i]; w.py=hy[i]+1; w.notice=false; w.dlg=false; step(1); for(int k=0;k<4&&w.notice;k++) step(5); } }
  { int n=0,k=item_index("OBJ_0034"); while(player().inv.has(k,n+1)) n++; printf("Herbes cueillies en foret: %d (attendu >=3)\n",n); }

  // --- Grand-Castel (loc10) ---
  world_scene().load_map(4); step(-1);   // gc_intro + quest_start(8)
  grant("OBJ_0014",1);                   // Pierre noire (boss mine) pour Q7
  // Q7 : Albin npc13 (9,8) -> 2 visites avec Pierre noire
  talk(9,9,1); talk(9,9,1);
  printf("Q7 (symbole Albin)    : %d  (Q13 lancee=%d)\n",Q(7),Q(13));
  // Q8 : Garrick npc15 (11,19) -> prend contrats + pli ; livraisons ; retour
  talk(11,18,2);                         // Garrick : q8_pris + pli
  // Tomas Venn npc16 (20,20) : pli
  talk(20,19,2);
  // Iris npc17 (18,19) : 3 herbes (cueillies en foret plus haut)
  talk(18,18,2);
  // Soeur Helena npc12 (5,7) : 1 potion (le joueur en a au depart)
  talk(5,8,1);
  // retour Garrick -> complete Q8
  talk(11,18,2);
  printf("Q8 (Guilde)           : %d  c_pli=%d c_herbes=%d c_aumone=%d\n",
      Q(8),flag_get("c_pli"),flag_get("c_herbes"),flag_get("c_aumone"));
  // Q15 : Meluve npc19 (6,8) start ; Racha npc18 (25,2) donne collier ; Meluve termine
  talk(6,9,1);          // Meluve : vu_meluve + start Q15
  talk(25,3,1);         // Racha : donne collier
  talk(6,9,1);          // Meluve : termine
  printf("Q15 (collier)         : %d  collier=%d\n",Q(15),player().inv.has(item_index("OBJ_0038"),1));

  printf("\n-- RECAP (0=pas lancee,1=en cours,2=terminee) --\n");
  int ok=0,tot=0; int check[]={0,1,2,3,4,7,8,15,16};
  for(int q:check){ tot++; bool done=(Q(q)==2); if(done)ok++; printf("  Q%-2d : %s\n",q,done?"OK (2)":"ECHEC"); }
  printf("=> %d/%d quetes testees terminees\n",ok,tot);
  return 0;
}
