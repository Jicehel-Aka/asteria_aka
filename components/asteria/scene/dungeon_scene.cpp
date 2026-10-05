// Donjon 1re personne (pseudo-3D grille, facon Dark & Under).
// Murs/sol/plafond textures pierre + eclairage par distance + torches vacillantes.
#include "scene/dungeon_scene.h"
#include "scene/combat_scene.h"
#include "scene/world_scene.h"
#include "platform/gb_port.h"
#include "ui/text.h"
#include "player.h"
#include "gamestate.h"
#include "generated/asteria_tiles.h"
#include "generated/asteria_monster.h"
#include "generated/asteria_dungeon_tex.h"
#include <cstdio>
#include <cstring>
#include <cmath>
namespace asteria {

static const int DX[4]={0,1,0,-1};   // 0=N 1=E 2=S 3=O
static const int DY[4]={-1,0,1,0};

DungeonScene& dungeon_scene(){ static DungeonScene s; return s; }
void dungeon_reset(){ dungeon_scene().gen=false; }
uint32_t DungeonScene::rnd(){ rng^=rng<<13; rng^=rng>>17; rng^=rng<<5; return rng; }

// cadres de projection (viewport 210x240, centre 105,120)
static const int LX[6]={0,44,70,84,93,98};
static const int RX[6]={210,166,140,126,117,112};
static const int TY[6]={0,50,80,97,106,112};
static const int BY[6]={240,190,160,143,134,128};
static const int VW=210, VH=240, CX=105, CY=120, MAXD=5;

// cellules : 0 sol, 1 mur, 2 filon, 3 mur a torche, 5 eau
static bool  inbg(int x,int y){ return x>=0&&y>=0&&x<DungeonScene::GW&&y<DungeonScene::GH; }
static int   cellAt(DungeonScene& d,int x,int y){ return inbg(x,y)? d.cell[y*DungeonScene::GW+x] : 1; }
static bool  isWallC(int c){ return c==1||c==2||c==3; }
static bool  isWall(DungeonScene& d,int x,int y){ return isWallC(cellAt(d,x,y)); }

static void vline(int x,int y0,int y1,gb::Color c){ if(y0>y1){int t=y0;y0=y1;y1=t;} for(int y=y0;y<y1;y++) gb::pixel(x,y,c); }

// ----- eclairage entier d'un texel : l=256 -> 1.0 -----
static inline gb::Color shade(uint16_t c,int l){
  int b=(c>>11)&31, g=(c>>5)&63, r=c&31; b=b*l>>8; g=g*l>>8; r=r*l>>8;
  if(b>31)b=31; if(g>63)g=63; if(r>31)r=31; return (gb::Color)((b<<11)|(g<<5)|r);
}
static void sparkle(int x0,int y0,int x1,int y1,uint32_t s){
  if(x1<=x0)x1=x0+1; if(y1<=y0)y1=y0+1; uint32_t r=(s*2654435761u)|1u;
  for(int i=0;i<7;i++){ r^=r<<13;r^=r>>17;r^=r<<5; int x=x0+(int)(r%(uint32_t)(x1-x0)); r^=r<<7; int y=y0+(int)(r%(uint32_t)(y1-y0));
    gb::pixel(x,y,gb::rgb(235,205,95)); gb::pixel(x,y+1,gb::rgb(150,120,40)); }
}
// bruit de roche lisse (value-noise) pour casser les aretes droites, stable par cellule
static int hsh(int v){ uint32_t x=(uint32_t)v*2654435761u; x^=x>>13; x*=2246822519u; x^=x>>16; return (int)(x&2047); }
static int rough(int seed,int i,int amp){ if(amp<1)amp=1; int a=hsh(seed+(i>>2)), b=hsh(seed+(i>>2)+1); int f=i&3; int va=a%amp, vb=b%amp; return va+((vb-va)*f)/4; }
static int vari(int wx,int wy){ return (wx*7+wy*13)&(dtex::NWALL-1); }

// ----- sol + plafond en floor-casting perspectif (dalles qui fuient correctement) -----
static void floorCeil(DungeonScene& d,float posX,float posY,float dirX,float dirY,float planeX,float planeY,float flick){
  const int TWt=dtex::TW, TWm=dtex::TW-1, THm=dtex::TH-1;
  const uint16_t* FL=dtex::FLOOR[0]; const uint16_t* CE=dtex::WALL[2];
  for(int y=CY+1;y<240;y++){
    int p=y-CY; float rowDist=(float)CY/p;
    float sX=rowDist*(2*planeX)/VW, sY=rowDist*(2*planeY)/VW;
    float fX=posX+rowDist*dirX-rowDist*planeX, fY=posY+rowDist*dirY-rowDist*planeY;
    float lf=1.18f-rowDist*0.17f; if(lf<0.14f)lf=0.14f; int l=(int)(lf*flick*256.f);
    for(int x=0;x<VW;x++){ int cx=(int)fX, cy=(int)fY; if(fX<0)cx--; if(fY<0)cy--;
      int tx=((int)(dtex::TW*(fX-cx)))&TWm, ty=((int)(dtex::TH*(fY-cy)))&THm;
      gb::Color c=shade(FL[ty*TWt+tx],l);
      if(cellAt(d,cx,cy)==5){ int b=(c>>11)&31,g=(c>>5)&63,r=c&31; b=b+(31-b)*3/5; g=g*3/5; r=r*2/5; c=(gb::Color)((b<<11)|(g<<5)|r); } // flaque
      gb::pixel(x,y,c); fX+=sX; fY+=sY; }
  }
  for(int y=0;y<CY;y++){
    int p=CY-y; if(p<1)p=1; float rowDist=(float)CY/p;
    float sX=rowDist*(2*planeX)/VW, sY=rowDist*(2*planeY)/VW;
    float fX=posX+rowDist*dirX-rowDist*planeX, fY=posY+rowDist*dirY-rowDist*planeY;
    float lf=0.78f-rowDist*0.16f; if(lf<0.10f)lf=0.10f; int l=(int)(lf*flick*256.f);
    for(int x=0;x<VW;x++){ int cx=(int)fX, cy=(int)fY; if(fX<0)cx--; if(fY<0)cy--;
      int tx=((int)(dtex::TW*(fX-cx)))&TWm, ty=((int)(dtex::TH*(fY-cy)))&THm;
      gb::pixel(x,y,shade(CE[ty*TWt+tx],l)); fX+=sX; fY+=sY; }
  }
}

// ----- flamme de torche vacillante -----
static void flame(int cx,int baseY,int h,float ph){
  if(h<3)h=3; int w=h*2/3; if(w<2)w=2;
  for(int y=0;y<h;y++){ float t=(float)y/h; int half=(int)((1.f-t)*w*0.6f+0.5f);
    int sway=(int)(sinf((baseY-y)*0.6f + ph*6.283f)*(t*2.2f));
    for(int x=-half;x<=half;x++){ int yy=baseY-y, xx=cx+x+sway;
      gb::Color c=(t<0.30f)?gb::rgb(255,242,185):(t<0.66f?gb::rgb(250,165,45):gb::rgb(205,72,20));
      gb::pixel(xx,yy,c); } }
  gb::pixel(cx,baseY-h/2,gb::rgb(255,252,235));
}
static void torch(int cx,int midY,int sz,float ph){
  if(sz<2)sz=2; if(sz>11)sz=11;                          // plafonne : pas de torche geante au premier plan
  gb::fill_rect(cx-1,midY,2,sz,gb::rgb(66,46,26));        // support/baton
  gb::fill_rect(cx-sz/3,midY+sz,(sz*2)/3+1,1,gb::rgb(40,30,18));
  int fh=sz + (int)(sz*0.8f*(0.55f+0.45f*sinf(ph*6.283f)));
  flame(cx,midY-1,fh,ph);
}

static void blit_scaled(const spr::Sprite& s,int dx,int dy,int dw,int dh){
  if(dw<1||dh<1)return; for(int y=0;y<dh;y++){ int sy=y*s.h/dh; for(int x=0;x<dw;x++){ int sx=x*s.w/dw;
    uint16_t c=s.px[sy*s.w+sx]; if(c!=spr::KEY) gb::pixel(dx+x,dy+y,c); } }
}
static void draw_line(int x0,int y0,int x1,int y1,gb::Color c){
  int dx=x1-x0,dy=y1-y0,ax=dx<0?-dx:dx,ay=dy<0?-dy:dy,sx=dx<0?-1:1,sy=dy<0?-1:1,e=ax-ay;
  for(;;){ gb::pixel(x0,y0,c); if(x0==x1&&y0==y1)break; int e2=2*e; if(e2>-ay){e-=ay;x0+=sx;} if(e2<ax){e+=ax;y0+=sy;} }
}
static void bar(int x,int y,int w,int h,int val,int mx,gb::Color fg,gb::Color bg){
  if(mx<1)mx=1; if(val<0)val=0; if(val>mx)val=mx; gb::fill_rect(x,y,w,h,bg); gb::fill_rect(x,y,val*w/mx,h,fg);
}

// ----- panneau droit -----
static void draw_panel(DungeonScene& d){
  const int PX=211;
  gb::fill_rect(PX,0,320-PX,240,gb::rgb(16,14,18)); vline(PX,0,240,gb::rgb(74,66,84));
  int ccx=PX+54, ccy=30, R=15; gb::fill_rect(PX+6,4,98,52,gb::rgb(26,22,30));
  const char* L[4]={"N","E","S","O"}; int lx[4]={ccx-3,ccx+R+2,ccx-3,ccx-R-9}; int ly[4]={ccy-R-11,ccy-6,ccy+R-2,ccy-6};
  for(int i=0;i<4;i++) ui::text(lx[i],ly[i],L[i], i==d.face?gb::rgb(242,212,110):gb::rgb(120,112,100));
  draw_line(ccx,ccy,ccx+DX[d.face]*R,ccy+DY[d.face]*R,gb::rgb(222,84,62));
  gb::fill_rect(ccx-1,ccy-1,3,3,gb::rgb(232,222,182));
  int yy=64; char b[32]; int pv=player().VIT-player().BL; if(pv<0)pv=0;
  ui::text(PX+6,yy,"PV",gb::rgb(200,200,180)); bar(PX+30,yy+2,68,6,pv,player().VIT,gb::rgb(80,180,90),gb::rgb(45,60,45)); yy+=15;
  ui::text(PX+6,yy,"PM",gb::rgb(200,200,180)); bar(PX+30,yy+2,68,6,player().mana,player().manaMax,gb::rgb(92,120,222),gb::rgb(40,44,70)); yy+=15;
  snprintf(b,32,"Or %d  Niv %d",player().gold,player().level); ui::text(PX+6,yy,b,gb::rgb(222,200,120)); yy+=16;
  const int GW=DungeonScene::GW, GH=DungeonScene::GH, mc=5; int mx=PX+(108-GW*mc)/2, my=yy;
  gb::fill_rect(mx-2,my-2,GW*mc+4,GH*mc+4,gb::rgb(10,9,12));
  for(int y=0;y<GH;y++)for(int x=0;x<GW;x++){ if(!d.seen[y*GW+x])continue; uint8_t t=d.cell[y*GW+x]; gb::Color c;
    if(t==1)c=gb::rgb(70,64,58); else if(t==2)c=gb::rgb(150,130,60); else if(t==3)c=gb::rgb(210,120,40); else if(t==5)c=gb::rgb(50,80,130); else c=gb::rgb(120,112,100);
    gb::fill_rect(mx+x*mc,my+y*mc,mc-1,mc-1,c); }
  if(d.seen[d.by*GW+d.bx]&&!flag_get("boss_mine")) gb::fill_rect(mx+d.bx*mc,my+d.by*mc,mc-1,mc-1,gb::rgb(205,62,50));
  gb::fill_rect(mx+d.px*mc,my+d.py*mc,mc-1,mc-1,gb::rgb(242,212,92));
  gb::fill_rect(mx+d.px*mc+2+DX[d.face]*2,my+d.py*mc+2+DY[d.face]*2,2,2,gb::rgb(255,255,255));
  int jy=my+GH*mc+8, jmaxw=320-(PX+5)-3;
  for(int i=0;i<d.jrn;i++){ char tmp[40]; strncpy(tmp,d.jr[i],39); tmp[39]=0; int n=(int)strlen(tmp);
    while(n>0 && ui::text_w(tmp)>jmaxw) tmp[--n]=0; ui::text(PX+5,jy+i*13,tmp,gb::rgb(172,164,142)); }
}

// ----- scene -----
void DungeonScene::journal(const char* s){
  if(jrn<3){ strncpy(jr[jrn],s,39); jr[jrn][39]=0; jrn++; }
  else { for(int i=0;i<2;i++) strncpy(jr[i],jr[i+1],40); strncpy(jr[2],s,39); jr[2][39]=0; }
}
void DungeonScene::mark_seen(){
  seen[py*GW+px]=1; const int nd[4][2]={{0,-1},{0,1},{1,0},{-1,0}};
  for(int i=0;i<4;i++){ int x=px+nd[i][0],y=py+nd[i][1]; if(inbg(x,y)) seen[y*GW+x]=1; }
  int cx=px,cy=py; for(int i=0;i<5;i++){ cx+=DX[face]; cy+=DY[face]; if(!inbg(cx,cy))break; seen[cy*GW+cx]=1; if(isWall(*this,cx,cy))break; }
}
void DungeonScene::generate(){
  for(int i=0;i<GW*GH;i++){ cell[i]=1; seen[i]=0; }
  int sx[GW*GH], sy[GW*GH], sp=0; cell[1*GW+1]=0; sx[sp]=1; sy[sp]=1; sp++;
  const int D4[4][2]={{0,-2},{0,2},{2,0},{-2,0}};
  while(sp>0){ int cx=sx[sp-1], cy=sy[sp-1]; int ord[4]={0,1,2,3};
    for(int i=3;i>0;i--){ int j=(int)(rnd()%(uint32_t)(i+1)); int t=ord[i];ord[i]=ord[j];ord[j]=t; }
    bool carved=false;
    for(int k=0;k<4;k++){ int ddx=D4[ord[k]][0], ddy=D4[ord[k]][1]; int nx=cx+ddx, ny=cy+ddy;
      if(nx>0&&ny>0&&nx<GW-1&&ny<GH-1 && cell[ny*GW+nx]==1){
        cell[(cy+ddy/2)*GW+(cx+ddx/2)]=0; cell[ny*GW+nx]=0; sx[sp]=nx; sy[sp]=ny; sp++; carved=true; break; } }
    if(!carved) sp--; }
  enx=1; eny=1; px=1; py=1; face=(cell[2*GW+1]==0)?2:1; bx=GW-2; by=GH-2;
  for(int yy=GH-4; yy<=GH-2; yy++) for(int xx=GW-4; xx<=GW-2; xx++) if(xx>0&&yy>0) cell[yy*GW+xx]=0;
  int placed=0;
  for(int t=0;t<300 && placed<12;t++){ int x=1+(int)(rnd()%(GW-2)), y=1+(int)(rnd()%(GH-2));
    if(cell[y*GW+x]==1 && (cell[y*GW+x-1]==0||cell[y*GW+x+1]==0||cell[(y-1)*GW+x]==0||cell[(y+1)*GW+x]==0)){ cell[y*GW+x]=2; placed++; } }
  int w=0;
  for(int t=0;t<300 && w<6;t++){ int x=1+(int)(rnd()%(GW-2)), y=1+(int)(rnd()%(GH-2));
    if(cell[y*GW+x]==0 && !(x==1&&y==1) && !(x>=GW-4&&y>=GH-4)){ cell[y*GW+x]=5; w++; } }
  int tc=0;                                   // torches murales
  for(int y=1;y<GH-1 && tc<16;y++)for(int x=1;x<GW-1;x++){ if(cell[y*GW+x]!=1)continue;
    bool adj=cell[y*GW+x-1]==0||cell[y*GW+x+1]==0||cell[(y-1)*GW+x]==0||cell[(y+1)*GW+x]==0;
    if(adj && (rnd()%6==0)){ cell[y*GW+x]=3; tc++; } }
  gen=true; steps=0; jrn=0; mark_seen();
  vx=px+0.5f; vy=py+0.5f; vdx=DX[face]; vdy=DY[face]; anim=false; pendEnc=false; at=0.f;
  journal("Mine humide."); journal("Chef au fond.");
}
void DungeonScene::enter(){ if(!gen){ rng=gb::millis()|1u; generate(); } }

static void exit_forest(SceneManager& m){ WorldScene& w=world_scene(); w.load_map(1); w.px=10; w.py=2; m.set(SceneId::WORLD); }
static void exit_gc(SceneManager& m){ WorldScene& w=world_scene(); w.load_map(4); w.px=15; w.py=22; m.set(SceneId::WORLD); }

void DungeonScene::update(SceneManager& m){
  if(anim){                                  // animation de pas / rotation en cours
    at+=0.22f;
    if(at>=1.f){ at=1.f; anim=false; vx=atx; vy=aty; vdx=atdx; vdy=atdy;
      if(animKind==1 && pendEnc){ pendEnc=false;
        if(cellAt(*this,px,py)==5) journal("Flaque d'eau.");
        if(steps>1 && (rnd()%5==0) && !(px==bx&&py==by)){ set_combat_return(SceneId::DUNGEON);
          start_combat((rnd()%2)?16:23,"/sdcard/ASTERIA/battle/MINE_L.BMP"); journal("Un monstre !"); m.set(SceneId::COMBAT); return; } }
    } else { float t=at, te=t*t*(3.f-2.f*t);     // smoothstep
      vx=afx+(atx-afx)*te; vy=afy+(aty-afy)*te;
      vdx=afdx+(atdx-afdx)*te; vdy=afdy+(atdy-afdy)*te;
      float n=sqrtf(vdx*vdx+vdy*vdy); if(n>1e-4f){vdx/=n;vdy/=n;} }
    return;
  }
  uint32_t p=gb::buttons_pressed();
  if((p&gb::BTN_LEFT)||(p&gb::BTN_RIGHT)){ int nf=(p&gb::BTN_LEFT)?(face+3)%4:(face+1)%4;
    anim=true; animKind=2; at=0.f; afx=atx=vx; afy=aty=vy; afdx=vdx; afdy=vdy; atdx=DX[nf]; atdy=DY[nf]; face=nf; return; }
  int mvx=0,mvy=0;
  if(p&gb::BTN_UP){ mvx=DX[face]; mvy=DY[face]; } else if(p&gb::BTN_DOWN){ mvx=-DX[face]; mvy=-DY[face]; }
  if(mvx||mvy){ int nx=px+mvx, ny=py+mvy;
    if(!isWall(*this,nx,ny)){ px=nx; py=ny; steps++; mark_seen();
      anim=true; animKind=1; at=0.f; afx=vx; afy=vy; atx=px+0.5f; aty=py+0.5f; afdx=atdx=vdx; afdy=atdy=vdy; pendEnc=true; }
    else journal("Un mur."); return; }
  if(p&gb::BTN_A){ int fx=px+DX[face], fy=py+DY[face];
    if(fx==bx&&fy==by && !flag_get("boss_mine")){ set_combat_return(SceneId::DUNGEON); start_combat(9,"/sdcard/ASTERIA/battle/MINE_L.BMP"); m.set(SceneId::COMBAT); return; }
    if(px==bx&&py==by && flag_get("boss_mine")){ exit_gc(m); return; }
    if(px==enx&&py==eny){ exit_forest(m); return; } }
  if(p&(gb::BTN_B|gb::BTN_MENU)){ exit_forest(m); return; }
}

// --- raycaster : 1 rayon/colonne, distance continue -> eclairage + hauteur + texture lisses ---
void DungeonScene::render(){
  uint32_t ms=gb::millis();
  float flick=1.f + 0.08f*sinf(ms*0.017f) + 0.05f*sinf(ms*0.041f) + 0.03f*sinf(ms*0.089f);
  if(flick<0.80f)flick=0.80f; if(flick>1.20f)flick=1.20f;
  float torchPh=(ms%1000)/1000.f;
  float dirX=vdx, dirY=vdy, planeX=-vdy*0.66f, planeY=vdx*0.66f, posX=vx, posY=vy;
  floorCeil(*this,posX,posY,dirX,dirY,planeX,planeY,flick);
  int tcx[16],tcy[16],tmin[16],tmax[16],tH[16],tn=0;
  for(int x=0;x<VW;x++){
    float camX=2.f*x/VW-1.f, rayX=dirX+planeX*camX, rayY=dirY+planeY*camX;
    int mapX=(int)posX, mapY=(int)posY;
    float ddX=(rayX==0.f)?1e30f:fabsf(1.f/rayX), ddY=(rayY==0.f)?1e30f:fabsf(1.f/rayY);
    int stepX,stepY; float sdX,sdY;
    if(rayX<0){stepX=-1; sdX=(posX-mapX)*ddX;} else {stepX=1; sdX=(mapX+1.f-posX)*ddX;}
    if(rayY<0){stepY=-1; sdY=(posY-mapY)*ddY;} else {stepY=1; sdY=(mapY+1.f-posY)*ddY;}
    int side=0, cell=1, guard=0;
    for(;;){ if(sdX<sdY){ sdX+=ddX; mapX+=stepX; side=0; } else { sdY+=ddY; mapY+=stepY; side=1; }
      cell=cellAt(*this,mapX,mapY); if(isWallC(cell)) break; if(++guard>40){cell=1;break;} }
    float perp=(side==0)?(sdX-ddX):(sdY-ddY); if(perp<0.08f)perp=0.08f;
    int H=(int)(VH/perp);
    float wallX=(side==0)?(posY+perp*rayY):(posX+perp*rayX); wallX-=floorf(wallX);
    int texX=(int)(wallX*dtex::TW); if((side==0&&rayX>0)||(side==1&&rayY<0)) texX=dtex::TW-1-texX;
    float lf=1.20f-perp*0.14f; if(side==1)lf*=0.82f; if(cell==3)lf+=0.5f; if(lf<0.12f)lf=0.12f; if(lf>1.5f)lf=1.5f;
    int l=(int)(lf*flick*256.f);
    const uint16_t* T=dtex::WALL[vari(mapX,mapY)];
    int amp=H/12; if(amp<1)amp=1; if(amp>8)amp=8; int seed=mapX*131+mapY*57;
    int ds=CY-H/2+rough(seed,texX,amp), de=CY+H/2-rough(seed+911,texX,amp);
    int Hc=de-ds; if(Hc<1){ ds=CY-H/2; de=CY+H/2; Hc=H; }
    int y0=ds<0?0:ds, y1=de>240?240:de;
    int texStep=(int)(((long long)dtex::TH<<16)/(Hc>0?Hc:1));
    int texPos=(y0-ds)*texStep;
    for(int y=y0;y<y1;y++){ int texY=(texPos>>16)&(dtex::TH-1); texPos+=texStep; gb::pixel(x,y,shade(T[texY*dtex::TW+texX],l)); }
    if(y0>0&&y0<240) gb::pixel(x,y0,gb::rgb(10,9,8));           // contour roche
    if(cell==2 && (x&3)==0) sparkle(x,y0,x+1,y1,mapX*53+mapY*17+texX);
    if(cell==3){ int fi=-1; for(int k=0;k<tn;k++) if(tcx[k]==mapX&&tcy[k]==mapY){fi=k;break;}
      if(fi<0){ if(tn<16){ fi=tn++; tcx[fi]=mapX; tcy[fi]=mapY; tmin[fi]=tmax[fi]=x; tH[fi]=H; } }
      else { if(x<tmin[fi])tmin[fi]=x; if(x>tmax[fi])tmax[fi]=x; if(H>tH[fi])tH[fi]=H; } }
  }
  for(int k=0;k<tn;k++){ int cx=(tmin[k]+tmax[k])/2, H=tH[k], top=CY-H/2; int sz=H/9; if(sz<2)sz=2; if(sz>11)sz=11; torch(cx,top+H/4,sz,torchPh); }
  // boss / escalier en billboard si la ligne droit devant est degagee
  int cxg=px,cyg=py,dist=0; bool clear=false;
  for(int s=1;s<=6;s++){ cxg+=DX[face]; cyg+=DY[face]; if(!inbg(cxg,cyg))break; if(cxg==bx&&cyg==by){clear=true;dist=s;break;} if(isWall(*this,cxg,cyg))break; }
  if(clear){ float pd=dist-0.3f; if(pd<0.5f)pd=0.5f; int lh=(int)(VH/pd), feet=CY+lh/2;
    if(!flag_get("boss_mine")){ const spr::Sprite& S=spr::MONSTER[9]; int dh=(int)(lh*0.85f); int dw=dh*S.w/S.h; blit_scaled(S,CX-dw/2,feet-dh,dw,dh); }
    else { int w=lh/4; gb::fill_rect(CX-w/2,CY-lh/3,w,lh/3,gb::rgb(205,195,120)); ui::text(CX-4,CY-lh/3+4,"^",gb::rgb(60,50,20)); } }
  draw_panel(*this);
}
}
