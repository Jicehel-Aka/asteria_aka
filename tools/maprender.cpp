// Outil : rend une carte ENTIERE en PPM (hors camera), avec PNJ. Reutilise les vraies fonctions autotile.
#include "platform/gb_port.h"
#include "autotile.h"
#include "generated/asteria_maps.h"
#include "generated/asteria_tiles.h"
#include "generated/asteria_hero.h"
#include "generated/asteria_npc.h"
#include <cstdio>
#include <cstdlib>
#include <vector>
using namespace asteria;
using namespace asteria::data;
namespace gb {
  static std::vector<uint16_t> FB; static int FBW=0,FBH=0;
  void pixel(int x,int y,Color c){ if((unsigned)x<(unsigned)FBW&&(unsigned)y<(unsigned)FBH) FB[y*FBW+x]=c; }
  void clear(Color c){ for(auto&v:FB)v=c; }
  void fill_rect(int x,int y,int w,int h,Color c){ for(int j=0;j<h;j++)for(int i=0;i<w;i++) pixel(x+i,y+j,c); }
}
static bool isAny(const Map&M,int x,int y,const int*set,int n){ if(x<0||y<0||x>=M.w||y>=M.h)return false; uint8_t q=M.tiles[y*M.w+x]; for(int i=0;i<n;i++) if(q==set[i])return true; return false; }
int main(int argc,char**argv){
  int mi=argc>1?atoi(argv[1]):0; const Map&M=MAPS[mi];
  gb::FBW=M.w*16; gb::FBH=M.h*16; gb::FB.assign(gb::FBW*gb::FBH,0);
  const int ROOF[]={7,9,10,11}, WALL[]={2,13,14,15}, WAT[]={3}, NAT[]={16}, PTH[]={1}, RMP[]={21};
  for(int y=0;y<M.h;y++)for(int x=0;x<M.w;x++){ uint8_t t=M.tiles[y*M.w+x]; int dx=x*16,dy=y*16;
    auto N=[&](const int*s,int n,int ox,int oy){return isAny(M,x+ox,y+oy,s,n);};
    if(t==3) draw_water_autotile(dx,dy,N(WAT,1,0,-1),N(WAT,1,0,1),N(WAT,1,1,0),N(WAT,1,-1,0),N(WAT,1,1,-1),N(WAT,1,-1,-1),N(WAT,1,1,1),N(WAT,1,-1,1));
    else if(t==16) draw_natural_water(dx,dy,N(NAT,1,0,-1),N(NAT,1,0,1),N(NAT,1,1,0),N(NAT,1,-1,0),N(NAT,1,1,-1),N(NAT,1,-1,-1),N(NAT,1,1,1),N(NAT,1,-1,1));
    else if(t==1) draw_natural_path(dx,dy,N(PTH,1,0,-1),N(PTH,1,0,1),N(PTH,1,1,0),N(PTH,1,-1,0),N(PTH,1,1,-1),N(PTH,1,-1,-1),N(PTH,1,1,1),N(PTH,1,-1,1));
    else if(t==7||t==9||t==10||t==11) draw_roof_tile(dx,dy,N(ROOF,4,0,-1),N(ROOF,4,0,1),N(ROOF,4,-1,0),N(ROOF,4,1,0),t);
    else if(t==2||t==13||t==14||t==15 || ((t==4||t==8||t==12) && ((x>0&&is_facade_tile(M.tiles[y*M.w+x-1]))||(x+1<M.w&&is_facade_tile(M.tiles[y*M.w+x+1]))))){
      const int FAC[]={2,13,14,15,4,8,12};
      bool fu=isAny(M,x,y-1,FAC,7),fd=isAny(M,x,y+1,FAC,7),fl=isAny(M,x-1,y,FAC,7),frr=isAny(M,x+1,y,FAC,7);
      int mat=facade_material(M.tiles,M.w,M.h,x,y);
      draw_facade_tile(dx,dy,fu,fd,fl,frr,t,mat); }
    else if(t==21) draw_rampart_tile(dx,dy,N(RMP,1,0,-1),N(RMP,1,0,1),N(RMP,1,-1,0),N(RMP,1,1,0));
    else if(t==6) draw_tree(dx,dy);
    else if(t==22) draw_tower(dx,dy);
    else if(t==24){ const int TB[]={24}; draw_table_tile(dx,dy, isAny(M,x,y-1,TB,1),isAny(M,x,y+1,TB,1),isAny(M,x-1,y,TB,1),isAny(M,x+1,y,TB,1)); }
    else if(t>=23&&t<=27) draw_inn_tile(dx,dy,t);
    else if(t<23){ const spr::Sprite&s=spr::TILE[t]; for(int j=0;j<16;j++)for(int i=0;i<16;i++) gb::pixel(dx+i,dy+j,s.px[j*16+i]); }
  }
  // PNJ
  for(int i=0;i<M.spawn_n;i++){ const Spawn&s=M.spawns[i]; if(s.npc>=0&&s.npc<spr::NPC_COUNT&&spr::NPC[s.npc].px){ const spr::Sprite&n=spr::NPC[s.npc];
    int bx=s.x*16+(16-n.w)/2, by=s.y*16+16-n.h+2; for(int j=0;j<n.h;j++)for(int i2=0;i2<n.w;i2++){ uint16_t c=n.px[j*n.w+i2]; if(c!=spr::KEY) gb::pixel(bx+i2,by+j,c);} } }
  // export PPM
  char nm[64]; snprintf(nm,sizeof(nm),"map_%d.ppm",mi); FILE*f=fopen(nm,"wb");
  fprintf(f,"P6\n%d %d\n255\n",gb::FBW,gb::FBH);
  for(int i=0;i<gb::FBW*gb::FBH;i++){ uint16_t c=gb::FB[i]; unsigned b=((c>>11)&0x1F)<<3,g=((c>>5)&0x3F)<<2,r=(c&0x1F)<<3; unsigned char px[3]={(unsigned char)r,(unsigned char)g,(unsigned char)b}; fwrite(px,1,3,f);} fclose(f);
  printf("%s %dx%d spawns=%d\n",nm,M.w,M.h,M.spawn_n); return 0;
}
