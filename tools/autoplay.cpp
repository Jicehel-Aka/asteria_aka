// Autopilote de validation : joue ASTERIA de bout en bout en pilotant le vrai moteur.
#include "scene/scene.h"
#include "scene/world_scene.h"
#include "scene/combat_scene.h"
#include "scene/dungeon_scene.h"
#include "platform/gb_port.h"
#include "generated/asteria_maps.h"
#include "player.h"
#include "gamestate.h"
#include "progress.h"
#include "audio/jingle.h"
#include <cstdio>
#include <cstdlib>
#include <queue>
#include <cstring>
#include <vector>
void host_set_budget(int n); void host_inject(int code); void host_tick();
using namespace asteria;
using data::MAPS; using data::TILE_SOLID; using data::Map;

static int curMapIndex(){ return world_scene().map; }
static const Map& CM(){ return MAPS[world_scene().map]; }
static bool isNpcCell(const Map& M,int x,int y){ for(int i=0;i<M.spawn_n;i++) if(M.spawns[i].x==x&&M.spawns[i].y==y) return true; return false; }
// coffres connus a traiter comme obstacles (loc -> liste de cellules)
static bool isChestCell(int loc,int x,int y){
  if(loc==8 && x==22 && y==7) return true;   // coffre foret (Q5)
  if(loc==0 && x==3  && y==21) return true;  // coffre Valbois
  if(loc==20&& x==4  && y==16) return true;  // coffre carnet
  return false;
}
static bool walk(const Map& M,int loc,int x,int y){
  if(x<0||y<0||x>=M.w||y>=M.h) return false;
  if(TILE_SOLID[M.tiles[y*M.w+x]]) return false;
  if(isNpcCell(M,x,y)) return false;
  if(isChestCell(loc,x,y)) return false;
  return true;
}
// BFS -> code direction (1U 2D 3L 4R) du 1er pas vers (tx,ty).
// adjacent=true : s'arrete sur une case VOISINE de (tx,ty). -1 si arrive/pas de chemin.
static int bfsDir(const Map& M,int loc,int sx,int sy,int tx,int ty,bool adjacent){
  auto goal=[&](int x,int y){ return adjacent ? (abs(x-tx)+abs(y-ty)==1) : (x==tx&&y==ty); };
  if(goal(sx,sy)) return -1;
  std::vector<int> prev(M.w*M.h,-1); std::vector<char> seen(M.w*M.h,0);
  std::queue<int> q; q.push(sy*M.w+sx); seen[sy*M.w+sx]=1;
  int dx[4]={0,0,-1,1}, dy[4]={-1,1,0,0}; int code[4]={1,2,3,4};
  int found=-1;
  while(!q.empty()){ int c=q.front(); q.pop(); int x=c%M.w,y=c/M.w;
    if(goal(x,y)){ found=c; break; }
    for(int k=0;k<4;k++){ int nx=x+dx[k],ny=y+dy[k];
      if(!walk(M,loc,nx,ny)) continue; int nc=ny*M.w+nx; if(seen[nc]) continue;
      seen[nc]=1; prev[nc]=c; q.push(nc); } }
  if(found<0) return -1;
  // remonter jusqu'au 1er pas
  int c=found; while(prev[c]!=sy*M.w+sx && prev[c]!=-1) c=prev[c];
  int x=c%M.w,y=c/M.w, ddx=x-sx, ddy=y-sy;
  for(int k=0;k<4;k++) if(dx[k]==ddx&&dy[k]==ddy) return code[k];
  return -1;
}
static int dirCodeToward(int sx,int sy,int tx,int ty){ // case adjacente -> code pour FAIRE FACE
  if(ty==sy-1) return 1; if(ty==sy+1) return 2; if(tx==sx-1) return 3; if(tx==sx+1) return 4; return -1;
}
// dir membre du WorldScene attendu pour une direction donnee (UP=1,DOWN=0,LEFT=2,RIGHT=3)
static int wdirFor(int code){ switch(code){case 1:return 1;case 2:return 0;case 3:return 2;case 4:return 3;} return -1; }

// ---- portail d'une map vers un loc cible ----
static bool portalCell(const Map& M,int targetLoc,int& px,int& py){
  for(int i=0;i<M.portal_n;i++) if(M.portals[i].target_loc==targetLoc){ px=M.portals[i].x; py=M.portals[i].y; return true; }
  return false;
}
static bool npcCell(const Map& M,int npc,int& px,int& py){
  for(int i=0;i<M.spawn_n;i++) if(M.spawns[i].npc==npc){ px=M.spawns[i].x; py=M.spawns[i].y; return true; }
  return false;
}

// Donjon : DX/DY par face (0N 1E 2S 3O)
static int DDX[4]={0,1,0,-1}, DDY[4]={-1,0,1,0};
static int dFaceFor(int dx,int dy){ for(int f=0;f<4;f++) if(DDX[f]==dx&&DDY[f]==dy) return f; return -1; }
// BFS grille donjon -> premiere case voisine sur le chemin vers (tx,ty) (ou sur la cible)
static int dBfsStep(DungeonScene& d,int sx,int sy,int tx,int ty,bool adjacent,int& nx,int& ny){
  const int GW=DungeonScene::GW,GH=DungeonScene::GH;
  auto wk=[&](int x,int y){ return x>=0&&y>=0&&x<GW&&y<GH && (d.cell[y*GW+x]==0||d.cell[y*GW+x]==5); };
  auto goal=[&](int x,int y){ return adjacent?(abs(x-tx)+abs(y-ty)==1):(x==tx&&y==ty); };
  if(goal(sx,sy)) return 0;
  std::vector<int> prev(GW*GH,-1); std::vector<char> seen(GW*GH,0);
  std::queue<int> q; q.push(sy*GW+sx); seen[sy*GW+sx]=1; int found=-1;
  int dx[4]={0,0,-1,1},dy[4]={-1,1,0,0};
  while(!q.empty()){ int c=q.front();q.pop(); int x=c%GW,y=c/GW; if(goal(x,y)){found=c;break;}
    for(int k=0;k<4;k++){ int ax=x+dx[k],ay=y+dy[k]; if(!wk(ax,ay))continue; int nc=ay*GW+ax; if(seen[nc])continue; seen[nc]=1;prev[nc]=c;q.push(nc);} }
  if(found<0) return -1;
  int c=found; while(prev[c]!=sy*GW+sx && prev[c]!=-1) c=prev[c];
  nx=c%GW; ny=c/GW; return 1;
}

int main(){
  audio::init(); host_set_budget(1000000000);
  new_game_reset();
  // MODE VALIDATION : on neutralise l'equilibrage du combat (stats gonflees) pour
  // isoler la VALIDATION DE LA PROGRESSION (verrous, declencheurs, quetes, bosses,
  // transitions). Le combat reste joue normalement ; seul l'enjeu de survie est retire.
  player().VIT=250; player().CO=16; player().MA=16;
  printf("== RUN COMPLET ASTERIA (mode validation : stats gonflees) ==\n");
  SceneManager mgr; mgr.set(SceneId::WORLD);   // Arc 0 : enter() charge la Grande Plaine
  int step=0; const char* MILE[]={
    "S0 parler au maire (vu_maire)","S1 entrer en foret","S2 coffre -> piste_mine",
    "S3 entrer dans la mine","S4 mine: boss + niv5 -> Route Royale","S5 vaincre Corbeau Noir",
    "S6 entrer a Grand-Castel","S7 Q7 Albin (Pierre noire)","S8 entrer dans l'Antre",
    "S9 vaincre le Gardien du Seuil","S10 vaincre le Coeur de Haine","S11 denouement (fin_arc1)"};
  bool done[12]={}; int lastStep=-1; int stuck=0; long tick=0; int lastLvl=player().level;
  auto loc=[&](){ return MAPS[world_scene().map].location; };

  for(tick=0; tick<2000000; tick++){
    // --- avancement des jalons ---
    if(step==0 && flag_get("vu_maire")) step=1;
    if(step==1 && loc()==8) step=2;
    if(step==2 && flag_get("piste_mine")) step=3;
    if(step==3 && mgr.current()==SceneId::DUNGEON) step=4;
    if(step==4 && mgr.current()==SceneId::WORLD && loc()==20) step=5;
    if(step==5 && flag_get("boss_corbeau")) step=6;
    if(step==6 && loc()==10) step=7;
    if(step==7 && quest_status(7)==2) step=8;
    if(step==8 && loc()==21) step=9;
    if(step==9 && flag_get("boss_gardien")) step=10;
    if(step==10 && flag_get("boss_coeur")) step=11;
    if(step==11 && flag_get("fin_arc1")){ printf("[t=%ld] >>> %s  OK\n",tick,MILE[11]); printf("\n*** RUN TERMINE : fin de l'Arc I atteinte ***\n"); break; }
    if(step!=lastStep){ if(lastStep>=0) printf("[t=%ld] >>> %s  OK (niv %d, or %d)\n",tick,MILE[lastStep],player().level,player().gold); printf("[t=%ld] -> %s\n",tick,MILE[step]); lastStep=step; stuck=0; }
    if(player().level!=lastLvl){ printf("[t=%ld]    (niveau %d)\n",tick,player().level); lastLvl=player().level; }

    SceneId sc=mgr.current();
    int inj=-1;

    if(sc==SceneId::GAMEOVER){ printf("[t=%ld] !!! GAME OVER a l'etape %s (niv %d) !!!\n",tick,MILE[step],player().level); break; }
    else if(sc==SceneId::COMBAT){ inj=5; }                 // A : attaque/avance
    else if(sc==SceneId::INN||sc==SceneId::SHOP||sc==SceneId::DICE){ inj=6; } // B : sortir
    else if(sc==SceneId::TITLE||sc==SceneId::CREATE||sc==SceneId::RULES){ inj=5; }
    else if(sc==SceneId::DUNGEON){
      DungeonScene& d=dungeon_scene();
      if(d.anim){ inj=-1; }
      else {
        int tx,ty; bool adjacent;
        if(player().level<5){ // farmer les rencontres : faire la navette entree <-> voisinage du boss
          static bool goBoss=false;
          if(d.px==d.enx&&d.py==d.eny) goBoss=true; else if(abs(d.px-d.bx)+abs(d.py-d.by)<=2) goBoss=false;
          if(goBoss){ tx=d.bx; ty=d.by; adjacent=true; } else { tx=d.enx; ty=d.eny; adjacent=false; }
        }
        else if(!flag_get("boss_mine")){ tx=d.bx; ty=d.by; adjacent=true; }   // niveau atteint -> frapper le boss
        else { tx=d.bx; ty=d.by; adjacent=false; }                            // sortie par le puits
        if(adjacent && abs(d.px-tx)+abs(d.py-ty)==1){
          int wantF=dFaceFor(tx-d.px,ty-d.py);
          if(d.face!=wantF){ int diff=(wantF-d.face+4)%4; inj=(diff==1)?4:3; }  // tourner (R=4,L=3)
          else inj=5;   // A : frapper le boss / ou (phase sortie) non-adjacent donc A sur la case
        } else if(!adjacent && d.px==tx && d.py==ty){ inj=5; } // sur la case boss -> A (sortie)
        else { int nx,ny,r=dBfsStep(d,d.px,d.py,tx,ty,adjacent,nx,ny);
          if(r==1){ int wantF=dFaceFor(nx-d.px,ny-d.py);
            if(d.face!=wantF){ int diff=(wantF-d.face+4)%4; inj=(diff==1)?4:(diff==3)?3:4; }
            else inj=1; }  // UP avance
          else inj=5; }
      }
    }
    else if(sc==SceneId::WORLD){
      WorldScene& w=world_scene();
      if(w.notice){ inj=5; }
      else if(w.dlg){ inj=5; }
      else {
        const Map& M=CM(); int L=loc();
        int tx=-1,ty=-1; bool adjacent=false; bool interact=false;
        if(L==22){ // Arc 0 : Grande Plaine -> Voyageur -> Valbois
          if(!flag_get("vu_voyageur")){ npcCell(M,21,tx,ty); adjacent=true; interact=true; }
          else { portalCell(M,0,tx,ty); } }
        else if(step==0){ if(!flag_get("anneau_recu")){ npcCell(M,20,tx,ty); adjacent=true; interact=true; } // Dame Orwenn : anneau
                     else { npcCell(M,0,tx,ty); adjacent=true; interact=true; } }               // maire
        else if(step==1){ portalCell(M,8,tx,ty); }
        else if(step==2){ tx=22; ty=7; adjacent=true; interact=true; }           // coffre foret
        else if(step==3){ portalCell(M,9,tx,ty); }
        else if(step==5){ portalCell(M,10,tx,ty); }                              // monter la route -> Grand-Castel (ambush Corbeau en chemin)
        else if(step==6){ portalCell(M,10,tx,ty); }
        else if(step==7){ npcCell(M,13,tx,ty); adjacent=true; interact=true; }   // Albin
        else if(step==8){                                                        // detour Vallee des Pierres (Q13) puis Antre
          if(quest_status(13)==1){
            if(L==23){ int mx=-1,my=0;
              if(!flag_get("pm6")){mx=5;my=14;} else if(!flag_get("pm12")){mx=11;my=9;}
              else if(!flag_get("pm22")){mx=17;my=7;} else if(!flag_get("pm36")){mx=22;my=12;}
              if(mx>=0){ tx=mx; ty=my; adjacent=true; interact=true; } else { portalCell(M,20,tx,ty); } }
            else if(L==20){ portalCell(M,23,tx,ty); }    // route -> Vallee
            else if(L==10){ portalCell(M,20,tx,ty); }    // Grand-Castel -> route
            else { portalCell(M,21,tx,ty); }
          } else {                                       // Q13 absente/finie -> cap sur l'Antre
            if(L==10){ portalCell(M,21,tx,ty); }
            else if(L==20){ portalCell(M,10,tx,ty); }
            else if(L==23){ portalCell(M,20,tx,ty); }
            else { portalCell(M,21,tx,ty); }
          }
        }
        else if(step>=9){ tx=10; ty=4; }                                         // autel de l'Antre
        if(tx<0){ inj=-1; }
        else if(step>=9 && w.py<=5 && w.px>=8 && w.px<=12){ inj=-1; } // a l'autel : laisser la logique se declencher
        else {
          int d=bfsDir(M,L,w.px,w.py,tx,ty,adjacent||interact);
          if(d==-1){ // arrive (adjacent) -> faire face puis interagir
            if(interact){ int f=dirCodeToward(w.px,w.py,tx,ty);
              if(f>0 && w.dir!=wdirFor(f)) inj=f; else inj=5; }
            else inj=-1;
          } else inj=d;
        }
      }
    }
    else inj=5;

    host_inject(inj);
    mgr.update();
    host_inject(-1);
    host_tick();
    // MODE VALIDATION : repos simule — on empeche la mort par accumulation de
    // blessures sur la longue serie de combats (sinon BL -> VIT sans auberge).
    if(player().BL > 150) player().BL = 0;

    // anti-blocage : se reinitialise sur tout progres (etape/niveau/scene/position)
    { static int pL=-1,pS=-1,pX=-1,pY=-1,pDx=-1,pDy=-1;
      int cS=(int)mgr.current(); int wx=world_scene().px, wy=world_scene().py;
      int dgx=dungeon_scene().px, dgy=dungeon_scene().py;
      if(player().level!=pL||cS!=pS||wx!=pX||wy!=pY||dgx!=pDx||dgy!=pDy){ stuck=0; pL=player().level;pS=cS;pX=wx;pY=wy;pDx=dgx;pDy=dgy; }
      else stuck++; }
    if(stuck>20000){ printf("[t=%ld] !!! BLOCAGE a l'etape %s (scene %d, loc %d, pos %d,%d) !!!\n",
        tick,MILE[step],(int)mgr.current(),loc(),world_scene().px,world_scene().py); break; }
  }
  printf("\nRESUME: etape finale=%d/11 (%s), niveau=%d, or=%d, ticks=%ld\n",
      step,MILE[step<12?step:11],player().level,player().gold,tick);
  // flags cles
  const char* F[]={"vu_maire","piste_mine","boss_mine","boss_corbeau","boss_gardien","boss_coeur","fin_arc1"};
  printf("flags:"); for(auto f:F) printf(" %s=%d",f,flag_get(f)?1:0); printf("\n");
  printf("Q7=%d  Clef(OBJ_0015)=%d  Pierre(OBJ_0014)=%d\n",quest_status(7),
      player().inv.has(item_index("OBJ_0015"),1),player().inv.has(item_index("OBJ_0014"),1));
  printf("Anneau(OBJ_0040)=%d  Q16=%d  Q13=%d  Q14=%d\n",
      player().inv.has(item_index("OBJ_0040"),1),quest_status(16),quest_status(13),quest_status(14));
  printf("Arc0: Q17=%d  Livre(OBJ_0041)=%d  vu_voyageur=%d\n",
      quest_status(17),player().inv.has(item_index("OBJ_0041"),1),flag_get("vu_voyageur"));
  return 0;
}
