#include "autotile.h"
#include "platform/gb_port.h"
#include "generated/asteria_water.h"
#include "generated/asteria_path.h"
#include "generated/asteria_tiles.h"
namespace asteria {
static void blit(const spr::Sprite& s,int x,int y){ for(int j=0;j<s.h;++j)for(int i=0;i<s.w;++i){ uint16_t c=s.px[j*s.w+i]; if(c!=spr::KEY) gb::pixel(x+i,y+j,c);} }
static void autotile(const spr::Sprite* S,int sx,int sy, bool n,bool s,bool e,bool w,bool ne,bool nw,bool se,bool sw){
  blit(S[0],sx,sy);
  if(!n) blit(S[1],sx,sy); if(!s) blit(S[2],sx,sy); if(!e) blit(S[3],sx,sy); if(!w) blit(S[4],sx,sy);
  if(n&&e&&!ne) blit(S[5],sx,sy); if(n&&w&&!nw) blit(S[6],sx,sy); if(s&&e&&!se) blit(S[7],sx,sy); if(s&&w&&!sw) blit(S[8],sx,sy);
}
void draw_water_autotile(int sx,int sy, bool n,bool s,bool e,bool w,bool ne,bool nw,bool se,bool sw){ autotile(spr::WATER,sx,sy,n,s,e,w,ne,nw,se,sw); }
void draw_path_autotile (int sx,int sy, bool n,bool s,bool e,bool w,bool ne,bool nw,bool se,bool sw){ autotile(spr::PATH ,sx,sy,n,s,e,w,ne,nw,se,sw); }

void draw_natural_water(int sx,int sy, bool n,bool s,bool e,bool w,bool ne,bool nw,bool se,bool sw){
  const spr::Sprite& G=spr::TILE[0];              // herbe dessous (coins arrondis -> herbe visible)
  for(int j=0;j<16;j++)for(int i=0;i<16;i++) gb::pixel(sx+i,sy+j,G.px[j*16+i]);
  const int R=6;
  for(int y=0;y<16;y++)for(int x=0;x<16;x++){
    bool inside=true;
    if(!n&&!e && x>=16-R && y<R){ int dx=x-(16-R), dy=(R-1)-y; if(dx*dx+dy*dy>(R-1)*(R-1)) inside=false; }
    if(!n&&!w && x<R && y<R){ int dx=(R-1)-x, dy=(R-1)-y; if(dx*dx+dy*dy>(R-1)*(R-1)) inside=false; }
    if(!s&&!e && x>=16-R && y>=16-R){ int dx=x-(16-R), dy=y-(16-R); if(dx*dx+dy*dy>(R-1)*(R-1)) inside=false; }
    if(!s&&!w && x<R && y>=16-R){ int dx=(R-1)-x, dy=y-(16-R); if(dx*dx+dy*dy>(R-1)*(R-1)) inside=false; }
    if(!inside) continue;
    uint16_t col=((x+y)%6==0)? gb::rgb(96,152,214) : gb::rgb(58,110,180);
    bool foam=false;
    if(!n&&y==0)foam=true; if(!s&&y==15)foam=true; if(!w&&x==0)foam=true; if(!e&&x==15)foam=true;
    // adoucir aussi juste sous l'arrondi
    if(foam) col=gb::rgb(190,224,242);
    gb::pixel(sx+x,sy+y,col);
  }
}

// ------- TOIT en pente (faîtage haut, avant-toit bas, pignons lateraux) -------
static void roofcols(int t, uint16_t& base, uint16_t& lite, uint16_t& dark, uint16_t& line){
  switch(t){
    case 9:  base=gb::rgb(70,100,170); lite=gb::rgb(130,160,220); dark=gb::rgb(44,66,120);  line=gb::rgb(52,78,138);  break;
    case 10: base=gb::rgb(205,120,55); lite=gb::rgb(240,170,100);dark=gb::rgb(150,80,35);  line=gb::rgb(165,95,40);  break;
    case 11: base=gb::rgb(90,150,90);  lite=gb::rgb(150,200,150);dark=gb::rgb(60,105,60);  line=gb::rgb(66,116,66);  break;
    default: base=gb::rgb(172,72,56);  lite=gb::rgb(215,125,105);dark=gb::rgb(120,45,38);  line=gb::rgb(140,56,44);  break;
  }
}
void draw_roof_tile(int sx,int sy, bool up,bool down,bool left,bool right, int t){
  uint16_t base,lite,dark,line; roofcols(t,base,lite,dark,line);
  // Texture de petites tuiles repetee (periode 4 px) -> AUCUNE couture entre cases.
  // Rangees de tuiles de 4px ; joints horizontaux + joints verticaux decales (ecailles).
  for(int y=0;y<16;y++)for(int x=0;x<16;x++){
    int ry=y&3;                      // position dans la rangee de tuiles
    int row=(y>>2);                  // numero de rangee (0..3)
    int off=(row&1)?2:0;             // decalage une rangee sur deux
    uint16_t c=base;
    if(ry==0) c=lite;                // reflet en haut de chaque tuile
    else if(ry==3) c=dark;           // ombre/joint en bas de chaque tuile
    if(((x+off)&3)==0 && ry!=0) c=line;  // joint vertical entre tuiles
    gb::pixel(sx+x,sy+y,c);
  }
  // Trim UNIQUEMENT aux bords reels du pan de toit :
  if(!up){ for(int x=0;x<16;x++){ gb::pixel(sx+x,sy+0,gb::rgb(250,246,228)); gb::pixel(sx+x,sy+1,lite); } } // faitage
  if(!down){ for(int x=0;x<16;x++) gb::pixel(sx+x,sy+15,gb::rgb(36,28,24)); }                               // avant-toit
  if(!left){ for(int y=0;y<16;y++) gb::pixel(sx+0,sy+y,dark); }                                             // pignon G
  if(!right){ for(int y=0;y<16;y++) gb::pixel(sx+15,sy+y,dark); }                                           // pignon D
}
// ------- MUR autotilé : texture (editable) + trim de coins/arêtes -------
void draw_wall_tile(int sx,int sy, bool up,bool down,bool left,bool right, int t){
  const spr::Sprite& W=spr::TILE[t];
  for(int y=0;y<16;y++)for(int x=0;x<16;x++) gb::pixel(sx+x,sy+y,W.px[y*16+x]);
  uint16_t sh=gb::rgb(44,40,46), pil=gb::rgb(165,165,175);
  if(!up){ for(int x=0;x<16;x++) gb::pixel(sx+x,sy+0,sh); }           // ombre sous avant-toit
  if(!left){ for(int y=0;y<16;y++){ gb::pixel(sx+0,sy+y,pil); gb::pixel(sx+1,sy+y,sh);} }   // pilastre G
  if(!right){ for(int y=0;y<16;y++){ gb::pixel(sx+15,sy+y,pil); gb::pixel(sx+14,sy+y,sh);} } // pilastre D
  if(!down){ for(int x=0;x<16;x++) gb::pixel(sx+x,sy+15,sh); }       // plinthe
}

// ------- REMPART de chateau : grosses pierres + creneaux (merlons/creneaux) + tours d'angle -------
void draw_rampart_tile(int sx,int sy, bool up,bool down,bool left,bool right){
  uint16_t st1=gb::rgb(122,120,114), st2=gb::rgb(100,98,94), mor=gb::rgb(72,70,66);
  uint16_t hi=gb::rgb(158,156,150), sh=gb::rgb(52,50,48);
  for(int y=0;y<16;y++)for(int x=0;x<16;x++){
    uint16_t c = (((x>>2)+(y>>2))&1)? st1 : st2;
    if((x&3)==0 || (y&3)==0) c=mor;            // joints de pierre
    gb::pixel(sx+x,sy+y,c);
  }
  bool corner=(!up&&!left)||(!up&&!right)||(!down&&!left)||(!down&&!right);
  if(corner){
    // TOUR RONDE d'angle : disque de pierre + liseré + merlons cardinaux
    int cx=8,cy=8,R=7;
    for(int y=0;y<16;y++)for(int x=0;x<16;x++){
      int dx=x-cx, dy=y-cy, d2=dx*dx+dy*dy;
      if(d2<=R*R){ uint16_t c=(((x>>1)+(y>>1))&1)? gb::rgb(140,136,128):gb::rgb(112,108,102);
        if(d2> (R-1)*(R-1)) c=gb::rgb(70,68,64);            // bord sombre
        if(dx<-2 && dy<-2 && d2<(R-2)*(R-2)) c=hi;          // reflet
        gb::pixel(sx+x,sy+y,c); }
    }
    // merlons (petits blocs clairs) aux 4 points cardinaux + diagonales
    const int mp[8][2]={{8,1},{8,14},{1,8},{14,8},{3,3},{12,3},{3,12},{12,12}};
    for(int k=0;k<8;k++){ int mx=mp[k][0],my=mp[k][1];
      gb::pixel(sx+mx,sy+my,hi); gb::pixel(sx+mx-1,sy+my,hi); gb::pixel(sx+mx,sy+my-1,hi); }
    return;
  }
  // Creneaux sur chaque bord exterieur (merlon clair 2px / creneau sombre 2px)
  for(int i=0;i<16;i++){ bool m=((i>>1)&1)==0; uint16_t top=m?hi:sh, second=m?st1:sh;
    if(!up){ gb::pixel(sx+i,sy+0,top); gb::pixel(sx+i,sy+1,second); }
    if(!down){ gb::pixel(sx+i,sy+15,top); gb::pixel(sx+i,sy+14,second); }
    if(!left){ gb::pixel(sx+0,sy+i,top); gb::pixel(sx+1,sy+i,second); }
    if(!right){ gb::pixel(sx+15,sy+i,top); gb::pixel(sx+14,sy+i,second); }
  }
}

// ------- ARBRE : grand houppier debordant vers le haut, tronc, base herbe -------
void draw_tree(int sx,int sy){
  const spr::Sprite& G=spr::TILE[0];                       // herbe dessous
  for(int y=0;y<16;y++)for(int x=0;x<16;x++) gb::pixel(sx+x,sy+y,G.px[y*16+x]);
  uint16_t trk=gb::rgb(96,64,34), trd=gb::rgb(72,48,26);
  for(int y=8;y<15;y++){ gb::pixel(sx+7,sy+y,trd); gb::pixel(sx+8,sy+y,trk); gb::pixel(sx+9,sy+y,trd); }
  uint16_t dk=gb::rgb(40,86,46), md=gb::rgb(56,112,60), lt=gb::rgb(80,150,86), ds=gb::rgb(30,66,36);
  // houppier : ellipse centree au-dessus du tronc, deborde jusqu'a ~ -13 px
  for(int yy=-14;yy<9;yy++)for(int xx=-2;xx<18;xx++){
    double nx=(xx-8)/9.5, ny=(yy-(-3))/11.0;
    if(nx*nx+ny*ny<=1.0){
      uint16_t c=md;
      if(((xx*3+yy*5))%7==0) c=dk;
      if(((xx*2+yy))%9==0) c=lt;
      if(yy> 4) c=ds;                                      // ombre basse
      gb::pixel(sx+xx,sy+yy,c);
    }
  }
}

// ------- TOUR ronde a toit conique (deborde vers le haut, style reference) -------
void draw_tower(int sx,int sy){
  uint16_t s1=gb::rgb(154,150,142), s2=gb::rgb(122,118,112), sd=gb::rgb(78,74,70), hi=gb::rgb(186,182,174);
  // corps rond en pierre (y 4..15)
  for(int y=4;y<16;y++){
    int hw = (y<6)? 5 : 6;                       // leger arrondi au sommet du corps
    for(int x=8-hw;x<=8+hw;x++){
      uint16_t c = (((x>>1)+(y>>1))&1)? s1 : s2;
      if(x==8-hw||x==8+hw) c=sd;                 // bord
      if(x<8-hw+2) c=hi;                         // reflet gauche
      gb::pixel(sx+x,sy+y,c);
    }
  }
  // bandeau de creneaux en haut du corps (y 4..5)
  for(int x=2;x<14;x++){ bool m=((x>>1)&1)==0; if(m){ gb::pixel(sx+x,sy+4,hi);} else { gb::pixel(sx+x,sy+4,sd);} }
  // toit conique : base y=4 (large) -> apex au-dessus de la case (y=-14)
  uint16_t r1=gb::rgb(158,96,52), r2=gb::rgb(126,74,40), rd=gb::rgb(96,54,28), rl=gb::rgb(186,120,70);
  int baseY=4, apexY=-14, baseHW=7;
  for(int y=baseY;y>=apexY;y--){
    int t = baseY - y;                           // 0..18
    int hw = baseHW - (baseHW*t)/(baseY-apexY);  // decroit jusqu'a 0
    for(int x=8-hw;x<=8+hw;x++){
      uint16_t c=r2;
      if(x<8-hw+2) c=rl; else if(x>8+hw-2) c=rd; // ombrage G clair / D sombre
      else if((x+y)&1) c=r1;
      gb::pixel(sx+x,sy+y,c);
    }
  }
  gb::pixel(sx+8,sy+apexY-1,gb::rgb(220,200,120)); // epi/finial dore
}

// ===================== FACADES : materiau unique + feature en surcouche =====================
// Un pan de facade = pierre(2)/brique(13)/bois(14)/crepi(15). La porte(4), la fenetre(8)
// et le panneau(12) sont dessines PAR-DESSUS le materiau, en couche transparente.
bool is_facade_tile(int t){ return t==2||t==13||t==14||t==15||t==4||t==8||t==12; }
static bool is_wall_mat(int t){ return t==2||t==13||t==14||t==15; }

// Materiau d'un pan contigu : on remonte a la cellule la plus a gauche puis on prend
// le 1er vrai mur rencontre -> toute la facade partage le meme materiau.
int facade_material(const unsigned char* M,int W,int H,int x,int y){
  int xs=x; while(xs-1>=0 && is_facade_tile(M[y*W+xs-1])) xs--;
  for(int xx=xs; xx<W && is_facade_tile(M[y*W+xx]); ++xx){ int q=M[y*W+xx]; if(is_wall_mat(q)) return q; }
  return 2; // defaut pierre
}

static void draw_window_overlay(int sx,int sy){
  uint16_t fr=gb::rgb(92,60,34), frHi=gb::rgb(128,88,52);
  uint16_t gl=gb::rgb(118,168,200), glHi=gb::rgb(176,210,228), sill=gb::rgb(198,192,180), sillSh=gb::rgb(120,112,98);
  int x0=4,x1=11,y0=3,y1=11;
  for(int y=y0;y<=y1;y++)for(int x=x0;x<=x1;x++) gb::pixel(sx+x,sy+y,gl);         // vitre
  for(int x=x0;x<=x1;x++){ gb::pixel(sx+x,sy+y0,glHi); }                           // reflet haut
  for(int y=y0;y<=y1;y++){ gb::pixel(sx+x0,sy+y,glHi); }                           // reflet gauche
  // cadre bois
  for(int x=x0-1;x<=x1+1;x++){ gb::pixel(sx+x,sy+y0-1,fr); gb::pixel(sx+x,sy+y1+1,fr); }
  for(int y=y0-1;y<=y1+1;y++){ gb::pixel(sx+x0-1,sy+y,fr); gb::pixel(sx+x1+1,sy+y,fr); }
  gb::pixel(sx+x0-1,sy+y0-1,frHi); gb::pixel(sx+x1+1,sy+y0-1,frHi);
  // meneaux (croix)
  int mx=(x0+x1)/2, my=(y0+y1)/2;
  for(int y=y0;y<=y1;y++){ gb::pixel(sx+mx,sy+y,fr); }
  for(int x=x0;x<=x1;x++){ gb::pixel(sx+x,sy+my,fr); }
  // appui de fenetre
  for(int x=x0-2;x<=x1+2;x++){ gb::pixel(sx+x,sy+y1+2,sill); gb::pixel(sx+x,sy+y1+3,sillSh); }
}

static void draw_door_overlay(int sx,int sy){
  uint16_t fr=gb::rgb(78,50,28), post=gb::rgb(100,66,38);
  uint16_t leaf=gb::rgb(120,80,44), seam=gb::rgb(78,50,28), leafHi=gb::rgb(150,104,60);
  uint16_t knob=gb::rgb(222,196,110);
  int x0=4,x1=11,top=2;
  // vantail
  for(int y=top+1;y<=15;y++)for(int x=x0;x<=x1;x++) gb::pixel(sx+x,sy+y,leaf);
  for(int y=top+1;y<=15;y++){ gb::pixel(sx+x0,sy+y,leafHi); }                      // reflet gauche
  // planches verticales
  for(int y=top+1;y<=15;y++){ gb::pixel(sx+6,sy+y,seam); gb::pixel(sx+9,sy+y,seam); }
  // chambranle + linteau
  for(int y=top;y<=15;y++){ gb::pixel(sx+x0-1,sy+y,post); gb::pixel(sx+x1+1,sy+y,post); }
  for(int x=x0-1;x<=x1+1;x++){ gb::pixel(sx+x,sy+top,fr); gb::pixel(sx+x,sy+top+1,fr); }
  // poignee
  gb::pixel(sx+x1-1,sy+9,knob); gb::pixel(sx+x1-1,sy+10,knob);
}

static void draw_sign_overlay(int sx,int sy){
  uint16_t arm=gb::rgb(60,50,40), board=gb::rgb(146,104,58), bd=gb::rgb(92,62,34), mark=gb::rgb(214,196,150);
  // potence depuis le haut
  for(int x=7;x<=13;x++) gb::pixel(sx+x,sy+1,arm);
  gb::pixel(sx+13,sy+2,arm); gb::pixel(sx+8,sy+2,arm); gb::pixel(sx+11,sy+2,arm);
  // panneau suspendu
  int x0=6,x1=13,y0=3,y1=10;
  for(int y=y0;y<=y1;y++)for(int x=x0;x<=x1;x++) gb::pixel(sx+x,sy+y,board);
  for(int x=x0;x<=x1;x++){ gb::pixel(sx+x,sy+y0,bd); gb::pixel(sx+x,sy+y1,bd); }
  for(int y=y0;y<=y1;y++){ gb::pixel(sx+x0,sy+y,bd); gb::pixel(sx+x1,sy+y,bd); }
  // petite marque
  for(int x=x0+2;x<=x1-2;x++) gb::pixel(sx+x,sy+(y0+y1)/2,mark);
  gb::pixel(sx+(x0+x1)/2,sy+y0+2,mark); gb::pixel(sx+(x0+x1)/2,sy+y1-2,mark);
}

// Dessine une cellule de facade : fond = materiau (avec memes bordures que draw_wall_tile),
// puis la feature en surcouche si selfTile est une porte/fenetre/panneau.
void draw_facade_tile(int sx,int sy, bool up,bool down,bool left,bool right, int selfTile,int matTile){
  draw_wall_tile(sx,sy, up,down,left,right, matTile);
  if(selfTile==8) draw_window_overlay(sx,sy);
  else if(selfTile==4) draw_door_overlay(sx,sy);
  else if(selfTile==12) draw_sign_overlay(sx,sy);
}
}
