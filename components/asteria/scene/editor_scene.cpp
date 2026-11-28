// Mode EDITEUR v2 : hub (carte / sprites), edition de carte (tuiles/depart/PNJ/portails/coffres)
// et editeur de sprites couleur (tuiles, heros, PNJ, monstres). Tout multilingue. Export fichiers.
#include "scene/editor_scene.h"
#include "scene/world_scene.h"   // draw_map_tile
#include <cstdarg>
#include "platform/gb_port.h"
#include "ui/text.h"
#include "i18n.h"
#include "generated/asteria_maps.h"
#include "generated/asteria_tiles.h"
#include "autotile.h"
#include "generated/asteria_hero.h"
#include "generated/asteria_npc.h"
#include "generated/asteria_monster.h"
#include <cstdio>
#include <cstring>
#include <cstdlib>
namespace asteria {
// SEC-04 : append borne (evite le sous-depassement size_t de sizeof(buf)-o et tout depassement)
static int app(char* buf,int cap,int o,const char* fmt,...){
  if(o>=cap-1) return cap-1;
  va_list ap; va_start(ap,fmt);
  int w=vsnprintf(buf+o,(size_t)(cap-o),fmt,ap);
  va_end(ap);
  if(w<0) return o;
  o+=w; if(o>=cap) o=cap-1;
  return o;
}
using namespace data;

// ============ etat general ============
static int g_screen=0;          // 0 hub, 1 carte, 2 sprite
static int g_hubSel=0;
static bool g_ov=false; static int g_ovSel=0;   // overlay actions

// ============ EDITEUR DE CARTE ============
static int   mp=0;
static uint8_t mtiles[64*64];
static int   mw=20, mh=15, mpx=0, mpy=0;
struct ESpawn{ int npc,x,y; };  static ESpawn msp[32]; static int mspn=0;
struct EPortal{ int x,y,target,ax,ay; }; static EPortal mpo[16]; static int mpon=0;
struct EChest{ int x,y; };     static EChest mch[16]; static int mchn=0;
static int mcx=0,mcy=0,mtool=0,mtile=1,mnpc=0,mtarget=0;
static const int EDIT_TILE_N=33;   // tuiles 0..32 (voir draw_map_tile)
static const int EDIT_LOC_N=24;    // lieux 0..23 (cibles de portail)
static char mmsg[56];

void editor_set_map(int m){ mp=m; }

static int sp_at(int x,int y){ for(int i=0;i<mspn;i++) if(msp[i].x==x&&msp[i].y==y) return i; return -1; }
static int po_at(int x,int y){ for(int i=0;i<mpon;i++) if(mpo[i].x==x&&mpo[i].y==y) return i; return -1; }
static int ch_at(int x,int y){ for(int i=0;i<mchn;i++) if(mch[i].x==x&&mch[i].y==y) return i; return -1; }

static void map_load(){
  const Map& M=MAPS[mp];
  if(M.w<=0||M.h<=0||M.w>64||M.h>64) return;                 // SEC-04 : mtiles est 64x64
  mw=M.w; mh=M.h; mpx=M.px; mpy=M.py; mcx=mcy=0;
  for(int i=0;i<mw*mh;i++) mtiles[i]=M.tiles[i];
  mspn=M.spawn_n; for(int i=0;i<M.spawn_n&&i<32;i++){ msp[i].npc=M.spawns[i].npc; msp[i].x=M.spawns[i].x; msp[i].y=M.spawns[i].y; }
  mpon=M.portal_n; for(int i=0;i<M.portal_n&&i<16;i++){ mpo[i].x=M.portals[i].x; mpo[i].y=M.portals[i].y; mpo[i].target=M.portals[i].target_loc; mpo[i].ax=M.portals[i].sx; mpo[i].ay=M.portals[i].sy; }
  mchn=0;
}

// Tampons d'export/relecture : alloues sur le tas (PSRAM sur AKA) le temps de
// l'operation. En statique (.bss) ils debordaient la DRAM interne de l'ESP32.
struct EdBuf{ char* p; int cap; explicit EdBuf(int n):p((char*)gb::alloc_big((unsigned)n)),cap(p?n:0){ if(p) p[0]=0; } ~EdBuf(){ if(p) gb::free_big(p); } };
static const int MAP_BUF=24576, SPR_BUF=32000;   // SEC-04 : 64x64 tuiles "32, " + PNJ/portails

// (voir EdBuf) SEC-04 : 64x64 tuiles * jusqu'a 4 octets ("32, ") = ~16 Ko + PNJ/portails -> 24 Ko.
static bool map_export(){
  EdBuf B(MAP_BUF); if(!B.p){ snprintf(mmsg,56,"Mémoire insuffisante"); return false; } char* buf=B.p; const int BUFN=B.cap; int o=0;
  o=app(buf,BUFN,o,"// MAP_%04d (editeur) start px=%d py=%d  w=%d h=%d\n",mp+1,mpx,mpy,mw,mh);
  o=app(buf,BUFN,o,"static const uint8_t MAP%d_T[]={",mp);
  for(int i=0;i<mw*mh;i++) o=app(buf,BUFN,o,"%s%d",i?", ":"",mtiles[i]);
  o=app(buf,BUFN,o,"};\nstatic const Spawn MAP%d_S[]={",mp);
  if(mspn==0) o=app(buf,BUFN,o,"{-1,0,0}");
  for(int i=0;i<mspn;i++) o=app(buf,BUFN,o,"%s{%d,%d,%d}",i?", ":"",msp[i].npc,msp[i].x,msp[i].y);
  o=app(buf,BUFN,o,"};\nstatic const Portal MAP%d_P[]={",mp);
  if(mpon==0) o=app(buf,BUFN,o,"{0,0,-1,-1,-1}");
  for(int i=0;i<mpon;i++) o=app(buf,BUFN,o,"%s{%d,%d,%d,%d,%d}",i?", ":"",mpo[i].x,mpo[i].y,mpo[i].target,mpo[i].ax,mpo[i].ay);
  o=app(buf,BUFN,o,"};\n// Coffres (a cabler dans world CHESTS) : ");
  for(int i=0;i<mchn;i++) o=app(buf,BUFN,o,"%s{loc,%d,%d}",i?", ":"",mch[i].x,mch[i].y);
  o=app(buf,BUFN,o,"\n");
  if(o>=(int)BUFN-1){ snprintf(mmsg,56,"Export tronqué !"); return false; }   // SEC-04 : jamais de fichier coupe
  return gb::write_text("/sdcard/ASTERIA/export_map.txt", buf);
}

static void map_reload(){
  EdBuf B(MAP_BUF); if(!B.p){ snprintf(mmsg,56,"Mémoire insuffisante"); return; } char* buf=B.p; int n=gb::read_text("/sdcard/ASTERIA/export_map.txt",buf,B.cap);
  if(n<=0){ snprintf(mmsg,56,"export_map.txt introuvable"); return; }
  char key[16]; snprintf(key,16,"MAP%d_T[]={",mp);
  char* p=strstr(buf,key); if(!p){ snprintf(mmsg,56,"MAP%d_T absent",mp); return; }
  p+=strlen(key); int i=0; int cap=mw*mh;
  while(*p && *p!='}' && i<cap){ while(*p==' '||*p==','){p++;} if(*p=='}'||!*p)break; { int v=atoi(p); mtiles[i++]=(uint8_t)((v>=0&&v<=32)?v:0); } while(*p&&*p!=','&&*p!='}')p++; }   // SEC-04 : valeur bornee
  snprintf(mmsg,56,"%s",i18n::tr(i18n::ED_RELOADED));
}

static void map_update(SceneManager& mgr){
  uint32_t p=gb::buttons_pressed();
  if(g_ov){
    static const int OV[4]={i18n::ED_OV_EXPORT,i18n::ED_OV_NEXTMAP,i18n::ED_OV_RELOAD,i18n::ED_OV_BACK};
    (void)OV;
    if(p&gb::BTN_DOWN) g_ovSel=(g_ovSel+1)%4;
    if(p&gb::BTN_UP)   g_ovSel=(g_ovSel+3)%4;
    if(p&(gb::BTN_B|gb::BTN_RUN)) g_ov=false;
    if(p&gb::BTN_A){ if(g_ovSel==0){ if(map_export()) snprintf(mmsg,56,"%s",i18n::tr(i18n::ED_EXPORTED)); }
      else if(g_ovSel==1){ mp=(mp+1)%MAP_COUNT; map_load(); }
      else if(g_ovSel==2){ map_reload(); }
      else { g_screen=0; } g_ov=false; }
    return;
  }
  if(p&gb::BTN_UP && mcy>0) mcy--;
  if(p&gb::BTN_DOWN && mcy<mh-1) mcy++;
  if(p&gb::BTN_LEFT && mcx>0) mcx--;
  if(p&gb::BTN_RIGHT && mcx<mw-1) mcx++;
  if(p&gb::BTN_A){
    if(mtool==0) mtiles[mcy*mw+mcx]=(uint8_t)mtile;
    else if(mtool==1){ mpx=mcx; mpy=mcy; }
    else if(mtool==2){ int s=sp_at(mcx,mcy); if(s>=0){ for(int k=s;k<mspn-1;k++) msp[k]=msp[k+1]; mspn--; } else if(mspn<32){ msp[mspn].npc=mnpc; msp[mspn].x=mcx; msp[mspn].y=mcy; mspn++; } }
    else if(mtool==3){ int s=po_at(mcx,mcy); if(s>=0){ for(int k=s;k<mpon-1;k++) mpo[k]=mpo[k+1]; mpon--; } else if(mpon<16){ mpo[mpon].x=mcx; mpo[mpon].y=mcy; mpo[mpon].target=mtarget; mpo[mpon].ax=-1; mpo[mpon].ay=-1; mpon++; } }
    else { int s=ch_at(mcx,mcy); if(s>=0){ for(int k=s;k<mchn-1;k++) mch[k]=mch[k+1]; mchn--; } else if(mchn<16){ mch[mchn].x=mcx; mch[mchn].y=mcy; mchn++; } }
  }
  if(p&gb::BTN_B){ if(mtool==0) mtile=(mtile+1)%EDIT_TILE_N; else if(mtool==2) mnpc=(mnpc+1)%spr::NPC_COUNT; else if(mtool==3) mtarget=(mtarget+1)%EDIT_LOC_N; }   // BUG-08 : toutes les tuiles / tous les PNJ
  if(p&gb::BTN_MENU) mtool=(mtool+1)%5;
  if(p&gb::BTN_RUN){ g_ov=true; g_ovSel=0; }
}

static int ecamx=0, ecamy=0;
static void map_render(){
  gb::clear(gb::rgb(10,10,14));
  ecamx=mcx*TILE_SIZE+TILE_SIZE/2-160; ecamy=mcy*TILE_SIZE+TILE_SIZE/2-120;
  int _mx=mw*TILE_SIZE-320,_my=mh*TILE_SIZE-240;
  if(_mx<0) ecamx=_mx/2; else { if(ecamx<0)ecamx=0; if(ecamx>_mx)ecamx=_mx; }
  if(_my<0) ecamy=_my/2; else { if(ecamy<0)ecamy=0; if(ecamy>_my)ecamy=_my; }
  // BUG-08 : meme rendu que le jeu (toutes les tuiles 0..32), limite a la zone visible.
  { data::Map V{"EDIT",0,(uint8_t)mw,(uint8_t)mh,(uint8_t)mpx,(uint8_t)mpy,mtiles,nullptr,0,nullptr,0};
    int x0=ecamx/TILE_SIZE-1, y0=ecamy/TILE_SIZE-1, x1=x0+23, y1=y0+18;
    if(x0<0)x0=0; if(y0<0)y0=0; if(x1>mw)x1=mw; if(y1>mh)y1=mh;
    gb::set_origin(ecamx,ecamy);
    for(int y=y0;y<y1;y++) for(int x=x0;x<x1;x++) draw_map_tile(V,x,y);
    gb::reset_raster(); }
  gb::fill_rect(mpx*TILE_SIZE+5-ecamx,mpy*TILE_SIZE+5-ecamy,6,6,gb::rgb(80,160,255));
  for(int i=0;i<mspn;i++) gb::fill_rect(msp[i].x*TILE_SIZE+4-ecamx,msp[i].y*TILE_SIZE+3-ecamy,8,10,gb::rgb(230,210,70));
  for(int i=0;i<mpon;i++){ int x=mpo[i].x*TILE_SIZE-ecamx,y=mpo[i].y*TILE_SIZE-ecamy; gb::fill_rect(x+2,y+2,TILE_SIZE-4,TILE_SIZE-4,gb::rgb(60,200,220)); }
  for(int i=0;i<mchn;i++){ int x=mch[i].x*TILE_SIZE-ecamx,y=mch[i].y*TILE_SIZE-ecamy; gb::fill_rect(x+3,y+5,10,8,gb::rgb(190,140,50)); }
  // curseur
  int _cx=mcx*TILE_SIZE-ecamx,_cy=mcy*TILE_SIZE-ecamy;
  gb::fill_rect(_cx,_cy,TILE_SIZE,2,gb::rgb(255,255,255));
  gb::fill_rect(_cx,_cy+TILE_SIZE-2,TILE_SIZE,2,gb::rgb(255,255,255));
  gb::fill_rect(_cx,_cy,2,TILE_SIZE,gb::rgb(255,255,255));
  gb::fill_rect(_cx+TILE_SIZE-2,_cy,2,TILE_SIZE,gb::rgb(255,255,255));
  // HUD haut
  gb::fill_rect(0,0,320,14,gb::rgb(16,14,22));
  const char* TN[5]={i18n::tr(i18n::ED_T_PAINT),i18n::tr(i18n::ED_T_START),i18n::tr(i18n::ED_T_SPAWN),i18n::tr(i18n::ED_T_PORTAL),i18n::tr(i18n::ED_T_CHEST)};
  char h[56];
  if(mtool==0)      snprintf(h,56,"%s %d  %s: %s  %s:%d",i18n::tr(i18n::ED_MAP),mp,i18n::tr(i18n::ED_TOOL),TN[0],i18n::tr(i18n::ED_TILE),mtile);
  else if(mtool==2) snprintf(h,56,"%s %d  %s: %s  PNJ:%d",i18n::tr(i18n::ED_MAP),mp,i18n::tr(i18n::ED_TOOL),TN[2],mnpc);
  else if(mtool==3) snprintf(h,56,"%s %d  %s: %s  loc:%d",i18n::tr(i18n::ED_MAP),mp,i18n::tr(i18n::ED_TOOL),TN[3],mtarget);
  else              snprintf(h,56,"%s %d  %s: %s",i18n::tr(i18n::ED_MAP),mp,i18n::tr(i18n::ED_TOOL),TN[mtool]);
  ui::text(4,2,h,gb::rgb(230,220,180));
  gb::fill_rect(0,214,320,26,gb::rgb(16,14,22));
  ui::text(4,216,i18n::tr(i18n::ED_HINT1),gb::rgb(190,185,160));
  ui::text(4,228,i18n::tr(i18n::ED_HINT2),gb::rgb(190,185,160));
  if(mmsg[0]){ gb::fill_rect(30,100,260,20,gb::rgb(20,40,20)); ui::gold(38,104,mmsg,true); }
  if(g_ov){
    const char* OV[4]={i18n::tr(i18n::ED_OV_EXPORT),i18n::tr(i18n::ED_OV_NEXTMAP),i18n::tr(i18n::ED_OV_RELOAD),i18n::tr(i18n::ED_OV_BACK)};
    gb::fill_rect(90,60,140,96,gb::rgb(20,16,28)); gb::fill_rect(90,60,140,2,gb::rgb(200,170,90));
    for(int i=0;i<4;i++){ int y=74+i*18; if(i==g_ovSel){ gb::fill_rect(94,y-2,132,16,gb::rgb(60,45,20)); ui::gold(98,y,">",true);} ui::text(110,y,OV[i],gb::rgb(235,225,190)); }
  }
}

// ============ EDITEUR DE SPRITES ============
// BUG-08 : tous les sprites de la banque (tailles reelles des tableaux generes).
static const int N_TILE=23, N_HERO=12, N_NPC=spr::NPC_COUNT, N_MON=(int)(sizeof(spr::MONSTER)/sizeof(spr::MONSTER[0]));
static const int ASSET_N=N_TILE+N_HERO+N_NPC+N_MON;
static uint16_t sbuf[64*64]; static int sw=16,sh=16;
static int sasset=0, scx=0, scy=0, scol=1;
static char smsg[48];
static const int PAL_N=17;
static uint16_t PAL(int i){
  static const uint8_t Cc[16][3]={ {0,0,0},{255,255,255},{128,128,128},{190,40,40},{40,130,50},{50,90,200},
    {235,200,80},{150,95,45},{95,65,125},{235,145,60},{70,185,185},{205,125,165},
    {120,165,65},{85,55,35},{205,195,175},{25,25,35} };
  if(i>=16) return spr::KEY;
  return gb::rgb(Cc[i][0],Cc[i][1],Cc[i][2]);
}
static const spr::Sprite& asset_of(int idx,int& cat,int& sub){
  if(idx<N_TILE){cat=0;sub=idx;return spr::TILE[idx];}
  idx-=N_TILE; if(idx<N_HERO){cat=1;sub=idx;return spr::HERO[idx/3][idx%3];}
  idx-=N_HERO; if(idx<N_NPC){cat=2;sub=idx;return spr::NPC[idx];}
  idx-=N_NPC; if(idx>=N_MON) idx=N_MON-1; {cat=3;sub=idx;return spr::MONSTER[idx];}
}
static void spr_load(){
  int c,s; const spr::Sprite& S=asset_of(sasset,c,s); scx=scy=0;
  if(!S.px){ sw=sh=16; for(int i=0;i<256;i++) sbuf[i]=spr::KEY; return; }
  sw=S.w>64?64:S.w; sh=S.h>64?64:S.h;
  for(int y=0;y<sh;y++)for(int x=0;x<sw;x++) sbuf[y*sw+x]=S.px[y*S.w+x];
}
static void spr_export(){
  EdBuf B(SPR_BUF); if(!B.p) return; char* buf=B.p; const int BUFN=B.cap; int o=0; int c,s; asset_of(sasset,c,s);
  const char* cat[4]={"tile","hero","npc","mon"};
  o=app(buf,BUFN,o,"// sprite edite : %s #%d  (%dx%d, 0xF81F=transparent)\nstatic const uint16_t EDIT_%s_%d[] = {\n  ",cat[c],s,sw,sh,cat[c],s);
  for(int i=0;i<sw*sh;i++){ o=app(buf,BUFN,o,"0x%04X%s",sbuf[i],(i<sw*sh-1)?",":""); if((i%16)==15) o=app(buf,BUFN,o,"\n  "); }
  o=app(buf,BUFN,o,"\n};\n");
  gb::write_text("/sdcard/ASTERIA/export_sprite.txt", buf);
}
static void spr_update(SceneManager& mgr){
  uint32_t p=gb::buttons_pressed();
  if(g_ov){
    if(p&gb::BTN_DOWN) g_ovSel=(g_ovSel+1)%3;
    if(p&gb::BTN_UP)   g_ovSel=(g_ovSel+2)%3;
    if(p&(gb::BTN_B|gb::BTN_RUN)) g_ov=false;
    if(p&gb::BTN_A){ if(g_ovSel==0){ spr_export(); snprintf(smsg,48,"%s",i18n::tr(i18n::ED_SPR_EXP)); }
      else if(g_ovSel==1){ sasset=(sasset+1)%ASSET_N; spr_load(); }
      else { g_screen=0; } g_ov=false; }
    return;
  }
  if(p&gb::BTN_UP && scy>0) scy--;
  if(p&gb::BTN_DOWN && scy<sh-1) scy++;
  if(p&gb::BTN_LEFT && scx>0) scx--;
  if(p&gb::BTN_RIGHT && scx<sw-1) scx++;
  if(p&gb::BTN_A) sbuf[scy*sw+scx]=PAL(scol);
  if(p&gb::BTN_B) scol=(scol+1)%PAL_N;
  if(p&gb::BTN_MENU){ sasset=(sasset+1)%ASSET_N; spr_load(); }
  if(p&gb::BTN_RUN){ g_ov=true; g_ovSel=0; }
}
static void spr_render(){
  gb::clear(gb::rgb(12,12,16));
  int zoom = (300/(sw?sw:1)); int zy=(176/(sh?sh:1)); if(zy<zoom)zoom=zy; if(zoom<1)zoom=1; if(zoom>12)zoom=12;
  int ox=(320 - sw*zoom)/2, oy=18;
  for(int y=0;y<sh;y++)for(int x=0;x<sw;x++){ uint16_t col=sbuf[y*sw+x]; int px=ox+x*zoom, py=oy+y*zoom;
    if(col==spr::KEY){ bool chk=((x+y)&1); gb::fill_rect(px,py,zoom,zoom, chk?gb::rgb(40,40,48):gb::rgb(28,28,34)); }
    else gb::fill_rect(px,py,zoom,zoom,col); }
  // curseur
  int cx=ox+scx*zoom, cy=oy+scy*zoom;
  gb::fill_rect(cx,cy,zoom,2,gb::rgb(255,80,80)); gb::fill_rect(cx,cy+zoom-2,zoom,2,gb::rgb(255,80,80));
  gb::fill_rect(cx,cy,2,zoom,gb::rgb(255,80,80)); gb::fill_rect(cx+zoom-2,cy,2,zoom,gb::rgb(255,80,80));
  // HUD haut
  gb::fill_rect(0,0,320,16,gb::rgb(16,14,22));
  int c,s; asset_of(sasset,c,s);
  const char* CAT[4]={i18n::tr(i18n::ED_CAT_TILE),i18n::tr(i18n::ED_CAT_HERO),i18n::tr(i18n::ED_CAT_NPC),i18n::tr(i18n::ED_CAT_MON)};
  char h[56]; snprintf(h,56,"%s #%d  %dx%d  %s:",CAT[c],s,sw,sh,i18n::tr(i18n::ED_COLOR));
  ui::text(4,3,h,gb::rgb(230,220,180));
  // palette en bas
  gb::fill_rect(0,196,320,44,gb::rgb(16,14,22));
  for(int i=0;i<PAL_N;i++){ int x=8+i*18, y=200; uint16_t col=PAL(i);
    if(i>=16){ gb::fill_rect(x,y,16,16,gb::rgb(40,40,48)); ui::text(x+3,y+3,"T",gb::rgb(200,200,210)); }
    else gb::fill_rect(x,y,16,16,col);
    if(i==scol){ gb::fill_rect(x-2,y-2,20,2,gb::rgb(255,255,255)); gb::fill_rect(x-2,y+16,20,2,gb::rgb(255,255,255)); gb::fill_rect(x-2,y-2,2,20,gb::rgb(255,255,255)); gb::fill_rect(x+16,y-2,2,20,gb::rgb(255,255,255)); } }
  ui::text(4,222,i18n::tr(i18n::ED_SPR_HINT),gb::rgb(190,185,160));
  if(smsg[0]){ gb::fill_rect(30,92,260,18,gb::rgb(20,40,20)); ui::gold(38,95,smsg,true); }
  if(g_ov){
    const char* OV[3]={i18n::tr(i18n::ED_OV_EXPORT),i18n::tr(i18n::ED_OV_NEXTSPR),i18n::tr(i18n::ED_OV_BACK)};
    gb::fill_rect(90,60,150,80,gb::rgb(20,16,28)); gb::fill_rect(90,60,150,2,gb::rgb(200,170,90));
    for(int i=0;i<3;i++){ int y=74+i*18; if(i==g_ovSel){ gb::fill_rect(94,y-2,142,16,gb::rgb(60,45,20)); ui::gold(98,y,">",true);} ui::text(110,y,OV[i],gb::rgb(235,225,190)); }
  }
}

// ============ HUB ============
EditorScene& editor_scene(){ static EditorScene s; return s; }
void EditorScene::enter(){ g_screen=0; g_hubSel=0; g_ov=false; mmsg[0]=0; smsg[0]=0; map_load(); spr_load(); }
void EditorScene::update(SceneManager& m){
  if(g_screen==1){ map_update(m); return; }
  if(g_screen==2){ spr_update(m); return; }
  uint32_t p=gb::buttons_pressed();
  if(p&gb::BTN_DOWN) g_hubSel=(g_hubSel+1)%3;
  if(p&gb::BTN_UP)   g_hubSel=(g_hubSel+2)%3;
  if(p&gb::BTN_B) m.set(SceneId::TITLE);
  if(p&gb::BTN_A){ if(g_hubSel==0){ g_screen=1; mmsg[0]=0; } else if(g_hubSel==1){ g_screen=2; smsg[0]=0; } else m.set(SceneId::TITLE); }
}
void EditorScene::render(){
  if(g_screen==1){ map_render(); return; }
  if(g_screen==2){ spr_render(); return; }
  gb::clear(gb::rgb(24,20,30));
  ui::text_big(90,24,i18n::tr(i18n::ED_TITLE),gb::rgb(230,210,140));
  const char* O[3]={i18n::tr(i18n::ED_HUB_MAP),i18n::tr(i18n::ED_HUB_SPR),i18n::tr(i18n::ED_OV_BACK)};
  for(int i=0;i<3;i++){ int y=90+i*26; if(i==g_hubSel){ gb::fill_rect(60,y-4,200,22,gb::rgb(60,45,20)); ui::gold(70,y,">",true);} ui::text(92,y,O[i],gb::rgb(235,225,190)); }
  ui::text(60,210,"A / B",gb::rgb(180,170,140));
}
}
