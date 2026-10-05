#include <cstdio>
#include "i18n.h"
#include "scene/world_scene.h"
#include "scene/dungeon_scene.h"
#include "ui/text.h"
#include "platform/gb_port.h"
#include "generated/asteria_maps.h"
#include "generated/asteria_gen.h"
#include "generated/asteria_hero.h"
#include "generated/asteria_tiles.h"
#include "autotile.h"
#include "generated/asteria_water.h"
#include "generated/asteria_npc.h"
#include "generated/asteria_dialog.h"
#include "gamestate.h"
#include "generated/asteria_quest.h"
#include "generated/asteria_event.h"
#include "scene/combat_scene.h"
#include "player.h"
namespace asteria {
using namespace data;
// couleurs de tuiles provisoires (avant sprites) : herbe,chemin,mur,eau,porte,pont,arbre,toit
static const gb::Color TCOL(uint8_t t){
  switch(t){ case 0:return gb::rgb(64,104,56); case 1:return gb::rgb(150,120,80);
    case 2:return gb::rgb(96,96,104); case 3:return gb::rgb(48,96,168); case 4:return gb::rgb(120,80,40);
    case 5:return gb::rgb(130,96,60); case 6:return gb::rgb(34,64,40); default:return gb::rgb(150,60,50);} }
struct ChestDef { int loc,x,y,item,gold; const char* flag; };
static const ChestDef CHESTS[]={ {8,16,10,4,8,"coffre_foret"}, {0,6,22,2,5,"coffre_valbois"} };
static const int NCHEST=2;
static int chest_at(int map,int x,int y){ int loc=MAPS[map].location; for(int i=0;i<NCHEST;i++) if(CHESTS[i].loc==loc && CHESTS[i].x==x && CHESTS[i].y==y && !flag_get(CHESTS[i].flag)) return i; return -1; }
static int map_for_loc(int loc);  // fwd map_for_loc
static bool door_leads_somewhere(const Map& M,int x,int y){ for(int i=0;i<M.portal_n;++i) if(M.portals[i].x==x&&M.portals[i].y==y) return map_for_loc(M.portals[i].target_loc)>=0; return false; }
static int npc_at(const Map& M,int x,int y){ for(int i=0;i<M.spawn_n;++i) if(M.spawns[i].x==x&&M.spawns[i].y==y) return M.spawns[i].npc; return -1; }
static int pick_node(const dlg::NpcDialog& D){ for(int i=0;i<D.nnodes;i++){ const char* r=D.nodes[i].require; bool ok; if(!r||!r[0]) ok=true; else if(r[0]=='!') ok=!flag_get(r+1); else ok=flag_get(r); if(ok) return i; } return D.nnodes>0?D.nnodes-1:0; }
WorldScene& world_scene(){ static WorldScene s; return s; }
void new_game_reset(){ WorldScene& w=world_scene(); w.started=false; w.confirmQuit=false; state_reset(); player()=new_caithness(); dungeon_reset(); }
static int map_for_loc(int loc){ for(int i=0;i<MAP_COUNT;++i) if(MAPS[i].location==loc) return i; return -1; }
void WorldScene::load_map(int m){ map=m; px=MAPS[m].px; py=MAPS[m].py; gb::log(MAPS[m].id); }
void WorldScene::enter(){
  if(started) return;            // retour de combat : on garde la carte/position
  started=true; load_map(0);
  flag_set("valbois_intro"); quest_start(0);
  notice=true; noticeTxt=evt::EVENTS[0].desc;
}
void WorldScene::update(SceneManager& mgr){
  const Map& M=MAPS[map]; uint32_t p=gb::buttons_pressed();
  if(MAPS[map].location==10 && !flag_get("gc_intro")){ flag_set("gc_intro"); quest_start(8); notice=true; noticeTxt="Les hautes tours de Grand-Castel se dressent devant toi."; }
  if(quest_status(0)==2 && quest_status(2)==0) quest_start(2);
  if(confirmQuit){ if(p&gb::BTN_A){ mgr.set(SceneId::TITLE); } else if(p&gb::BTN_B){ confirmQuit=false; } return; }
  if(notice){ if(p&(gb::BTN_A|gb::BTN_B)) notice=false; return; }
  if(dlg){ const dlg::Node& N=dlg::DIALOG[dlgNpc].nodes[dlgNode];
    if(p&gb::BTN_A){ dlgLine++; if(dlgLine>=N.nlines){ if(N.set_flag) flag_set(N.set_flag); if(N.give_item>=0) player().inv.add(N.give_item,1); if(N.complete_quest>=0) quest_complete(N.complete_quest); dlg=false; } }
    if(p&gb::BTN_B) dlg=false; return; }
  int nx=px,ny=py;
  if(p&gb::BTN_UP){ny=py-1;dir=1;} if(p&gb::BTN_DOWN){ny=py+1;dir=0;}
  if(p&gb::BTN_LEFT){nx=px-1;dir=2;} if(p&gb::BTN_RIGHT){nx=px+1;dir=3;} if(p&(gb::BTN_UP|gb::BTN_DOWN|gb::BTN_LEFT|gb::BTN_RIGHT)) lastMove=gb::millis();
  if(nx!=px||ny!=py){
    if(nx>=0&&ny>=0&&nx<M.w&&ny<M.h){
      uint8_t t=M.tiles[ny*M.w+nx];
      if(!TILE_SOLID[t] && npc_at(M,nx,ny)<0 && chest_at(map,nx,ny)<0 && !(t==4 && !door_leads_somewhere(M,nx,ny))){ px=nx; py=ny; bool _tp=false;
        for(int i=0;i<M.portal_n;++i){ const Portal& pt=M.portals[i];
          if(pt.x==px&&pt.y==py){
            if(pt.target_loc==9){ mgr.set(SceneId::DUNGEON); return; }   // la mine est un donjon 1re personne
            int nm=map_for_loc(pt.target_loc);
            if(nm>=0){ load_map(nm); if(pt.sx>=0){px=pt.sx;py=pt.sy;} _tp=true; }
            else gb::log("WORLD: portail (zone sans carte encore)");
          } }
        { int _loc=MAPS[map].location;
          if(!_tp && _loc==9 && py<=3 && !flag_get("boss_mine")){ set_combat_return(SceneId::WORLD); start_combat(9,"/sdcard/ASTERIA/battle/MINE_L.BMP"); mgr.set(SceneId::COMBAT); return; }
          if(!_tp && (_loc==8||_loc==9) && (gb::millis()%9==0)){ int _m=(_loc==8)?5+(int)((gb::millis()/3)%4):((gb::millis()%2)?16:23); set_combat_return(SceneId::WORLD); start_combat(_m, _loc==8?"/sdcard/ASTERIA/battle/FORE_J.BMP":"/sdcard/ASTERIA/battle/MINE_L.BMP"); mgr.set(SceneId::COMBAT); return; } }
      }
    }
  }
  if(p&gb::BTN_A){ int fx=px,fy=py; if(dir==0)fy++; else if(dir==1)fy--; else if(dir==2)fx--; else fx++;
    int n=npc_at(M,fx,fy);
    int _loc=MAPS[map].location;
    if(_loc==1 && n==13){ mgr.set(SceneId::INN); }
    else if(_loc==1 && n==10){ mgr.set(SceneId::DICE); }
    else if(n==2){ mgr.set(SceneId::SHOP); }
    else if(n>=0 && n<18 && dlg::DIALOG[n].nnodes>0){ dlg=true; dlgNpc=n; dlgNode=pick_node(dlg::DIALOG[n]); dlgLine=0; }
    else { int ci=chest_at(map,fx,fy);
      if(ci>=0){ const ChestDef& c=CHESTS[ci]; player().inv.add(c.item,1); player().gold+=c.gold; flag_set(c.flag); notice=true; noticeTxt="Un coffre ! Tu recuperes du butin."; }
      else if(fx>=0&&fy>=0&&fx<M.w&&fy<M.h && M.tiles[fy*M.w+fx]==4 && !door_leads_somewhere(M,fx,fy)){ notice=true; noticeTxt=i18n::tr(i18n::W_CLOSED); } } }
  if(p&(gb::BTN_MENU|gb::BTN_B)){ confirmQuit=true; }
}
void WorldScene::render(){
  const Map& M=MAPS[map];
  int camx=px*TILE_SIZE+TILE_SIZE/2-160, camy=py*TILE_SIZE+TILE_SIZE/2-120;
  int maxx=M.w*TILE_SIZE-320, maxy=M.h*TILE_SIZE-240;
  if(maxx<0) camx=maxx/2; else { if(camx<0)camx=0; if(camx>maxx)camx=maxx; }
  if(maxy<0) camy=maxy/2; else { if(camy<0)camy=0; if(camy>maxy)camy=maxy; }
  int x0=camx/TILE_SIZE-1, y0=camy/TILE_SIZE-1, x1=x0+25, y1=y0+20;
  if(x0<0)x0=0; if(y0<0)y0=0; if(x1>M.w)x1=M.w; if(y1>M.h)y1=M.h;
  for(int y=y0;y<y1;++y){ for(int x=x0;x<x1;++x){ uint8_t _t=M.tiles[y*M.w+x]; int dx=x*TILE_SIZE-camx, dy=y*TILE_SIZE-camy;
      if(_t==3){ auto W=[&](int xx,int yy){ if(xx<0||yy<0||xx>=M.w||yy>=M.h) return false; return M.tiles[yy*M.w+xx]==3; };
        draw_water_autotile(dx,dy, W(x,y-1),W(x,y+1),W(x+1,y),W(x-1,y), W(x+1,y-1),W(x-1,y-1),W(x+1,y+1),W(x-1,y+1)); }
      else if(_t==1){ auto P=[&](int xx,int yy){ if(xx<0||yy<0||xx>=M.w||yy>=M.h) return false; return M.tiles[yy*M.w+xx]==1; };
        draw_path_autotile(dx,dy, P(x,y-1),P(x,y+1),P(x+1,y),P(x-1,y), P(x+1,y-1),P(x-1,y-1),P(x+1,y+1),P(x-1,y+1)); }
      else if(_t==16){ auto N=[&](int xx,int yy){ if(xx<0||yy<0||xx>=M.w||yy>=M.h) return false; return M.tiles[yy*M.w+xx]==16; };
        draw_natural_water(dx,dy, N(x,y-1),N(x,y+1),N(x+1,y),N(x-1,y), N(x+1,y-1),N(x-1,y-1),N(x+1,y+1),N(x-1,y+1)); }
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
      else if(_t<23) gb::draw_image(spr::TILE[_t].px,spr::TILE[_t].w,spr::TILE[_t].h,dx,dy);
      else gb::fill_rect(dx,dy,TILE_SIZE,TILE_SIZE,TCOL(_t)); }
    // Tri par profondeur : les entites de la rangee y sont dessinees APRES ses tuiles.
    // Un arbre/tour d'une rangee inferieure (en avant) sera dessine ensuite -> il masque le heros, qui "passe derriere".
    for(int i=0;i<M.spawn_n;++i){ const Spawn& s=M.spawns[i]; if(s.y!=y) continue;
      if(s.npc>=0 && s.npc<spr::NPC_COUNT && spr::NPC[s.npc].px){ const spr::Sprite& ns=spr::NPC[s.npc]; gb::draw_image_key(ns.px,ns.w,ns.h,s.x*TILE_SIZE+(TILE_SIZE-ns.w)/2-camx,s.y*TILE_SIZE+TILE_SIZE-ns.h+2-camy,spr::KEY); } else gb::fill_rect(s.x*TILE_SIZE+4-camx,s.y*TILE_SIZE+2-camy,8,12,gb::rgb(230,220,120)); }
    if(py==y){ int fr = (gb::millis()-lastMove < 300) ? (int)((gb::millis()/120)%3) : 1; const spr::Sprite& hs=spr::HERO[dir][fr]; int sx=px*TILE_SIZE+(TILE_SIZE-hs.w)/2-camx; int sy=py*TILE_SIZE+TILE_SIZE-hs.h+2-camy; gb::draw_image_key(hs.px,hs.w,hs.h,sx,sy,spr::KEY); }
  }
  // Securite : heros hors de la plage balayee (ne devrait pas arriver, camera centree).
  if(py<y0||py>=y1){ int fr = (gb::millis()-lastMove < 300) ? (int)((gb::millis()/120)%3) : 1; const spr::Sprite& hs=spr::HERO[dir][fr]; int sx=px*TILE_SIZE+(TILE_SIZE-hs.w)/2-camx; int sy=py*TILE_SIZE+TILE_SIZE-hs.h+2-camy; gb::draw_image_key(hs.px,hs.w,hs.h,sx,sy,spr::KEY); }
  ui::gold(6,2,M.id,true);
  if(dlg){ const dlg::NpcDialog& D=dlg::DIALOG[dlgNpc];
    gb::fill_rect(8,158,304,74,gb::rgb(20,16,10));
    gb::fill_rect(8,158,304,2,gb::rgb(200,170,90)); gb::fill_rect(8,230,304,2,gb::rgb(200,170,90));
    ui::gold(16,162,D.name,true);
    const dlg::Node& _N=D.nodes[dlgNode]; const char* t=(dlgLine<_N.nlines)?_N.lines[dlgLine]:"";
    // wrap simple ~38 c
    char l1[64]={0},l2[64]={0}; int n=0; while(t[n])n++;
    if(n<=38){ for(int i=0;i<n;i++)l1[i]=t[i]; }
    else { int cut=38; while(cut>0&&t[cut]!=' ')cut--; if(cut==0)cut=38;
      for(int i=0;i<cut;i++)l1[i]=t[i]; int k=0; for(int i=cut+1;t[i];i++)l2[k++]=t[i]; }
    ui::text(16,182,l1,gb::rgb(236,226,192)); if(l2[0]) ui::text(16,198,l2,gb::rgb(236,226,192));
    ui::text(192,214,i18n::tr(i18n::W_DLG_H),gb::rgb(170,160,132));
  }
  for(int ci=0;ci<NCHEST;ci++){ if(chest_at(map,CHESTS[ci].x,CHESTS[ci].y)==ci){ int cx=CHESTS[ci].x*TILE_SIZE-camx, cy=CHESTS[ci].y*TILE_SIZE-camy;
      gb::fill_rect(cx+2,cy+5,12,9,gb::rgb(120,78,36)); gb::fill_rect(cx+2,cy+5,12,3,gb::rgb(200,170,80)); gb::fill_rect(cx+7,cy+8,2,3,gb::rgb(230,205,90)); } }
  { int qi=-1;
    for(int i=0;i<quest::QUEST_COUNT;i++){ if(quest_status(i)==1){qi=i;break;} }
    if(qi<0) for(int i=0;i<quest::QUEST_COUNT;i++){ if(quest_status(i)==2){qi=i;break;} }
    if(qi>=0){ int qs=quest_status(qi);
      gb::fill_rect(150,2,166,30,gb::rgb(20,16,10));
      ui::text(156,5,i18n::tr(i18n::W_QUEST),gb::rgb(200,180,110));
      ui::text(156,18,quest::QUEST[qi].name,gb::rgb(230,220,180));
      if(qs==1){ int o=quest_obj(qi); if(o>=quest::QUEST[qi].nobj)o=quest::QUEST[qi].nobj-1;
        gb::fill_rect(8,2,120,14,gb::rgb(20,16,10)); ui::gold(10,4,">",true); ui::text(20,4,quest::QUEST[qi].obj[o],gb::rgb(220,210,180)); }
      else if(qs==2){ gb::fill_rect(8,2,120,14,gb::rgb(20,16,10)); ui::gold(10,4,i18n::tr(i18n::W_QDONE),true); }
    }
  }
  if(notice){ gb::fill_rect(8,158,304,74,gb::rgb(20,16,10)); gb::fill_rect(8,158,304,2,gb::rgb(200,170,90)); gb::fill_rect(8,230,304,2,gb::rgb(200,170,90));
    ui::gold(16,162,i18n::tr(i18n::W_EVENT),true);
    // wrap ~38
    const char* t=noticeTxt?noticeTxt:""; char a[64]={0},b[64]={0}; int n=0; while(t[n])n++;
    if(n<=38){ for(int i=0;i<n;i++)a[i]=t[i]; } else { int cut=38; while(cut>0&&t[cut]!=' ')cut--; if(!cut)cut=38; for(int i=0;i<cut;i++)a[i]=t[i]; int k=0; for(int i=cut+1;t[i];i++)b[k++]=t[i]; }
    ui::text(16,182,a,gb::rgb(236,226,192)); if(b[0])ui::text(16,198,b,gb::rgb(236,226,192));
    ui::text(206,214,i18n::tr(i18n::W_CONT),gb::rgb(170,160,132));
  }
  if(confirmQuit){ gb::fill_rect(56,88,208,64,gb::rgb(20,16,10)); gb::fill_rect(56,88,208,2,gb::rgb(200,170,90)); gb::fill_rect(56,150,208,2,gb::rgb(200,170,90));
    ui::gold(74,100,i18n::tr(i18n::W_QUIT_Q),true);
    ui::text(74,124,i18n::tr(i18n::W_YESNO),gb::rgb(230,220,180)); }
}
}
