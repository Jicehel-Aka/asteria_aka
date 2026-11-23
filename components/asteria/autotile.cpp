// ASTERIA (Gamebuino AKA) - Rendu procedural des tuiles (herbe/eau/chemin/route/toits/murs/arbres) et auto-raccord des bordures selon les 8 voisins.
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

static inline int hash2(int a,int b);   // defini plus bas (meme unite de compilation)
// Bruit 2D lisse ~[-1,1] (coordonnees ABSOLUES, continu entre tuiles) : sert a faire
// ONDULER la rive sur plusieurs pixels -> casse les marches a 90 deg de la grille.
static inline double vnoise(int ax,int ay){
  const int S=7;
  auto fl=[&](int v){ return v>=0? v/S : -(((-v)+S-1)/S); };
  int x0=fl(ax), y0=fl(ay);
  double fx=(double)(ax-x0*S)/S, fy=(double)(ay-y0*S)/S;
  auto H=[&](int X,int Y)->double{ uint32_t u=(uint32_t)(X*374761393 + Y*668265263); u^=u>>13; u*=1274126177u; u^=u>>16; return (double)(u%1000)/500.0-1.0; };
  double a=H(x0,y0),b=H(x0+1,y0),c=H(x0,y0+1),d=H(x0+1,y0+1);
  fx=fx*fx*(3-2*fx); fy=fy*fy*(3-2*fy);
  double t=a+(b-a)*fx, bo=c+(d-c)*fx; return t+(bo-t)*fy;
}
// Couverture lisse [0,1] par interpolation bilineaire de l'occupation 3x3 (self=1).
// Les coins concaves/convexes sont adoucis -> les coudes se COURBENT (plus d'angles droits).
static inline double cov8(int x,int y, bool n,bool s,bool e,bool w,bool ne,bool nw,bool se,bool sw){
  double g[3][3]={ {nw?1.0:0,n?1.0:0,ne?1.0:0}, {w?1.0:0,1.0,e?1.0:0}, {sw?1.0:0,s?1.0:0,se?1.0:0} };
  double xc=x+0.5, yc=y+0.5; int cx0,cx1,cy0,cy1; double tx,ty;
  if(xc<8){cx0=0;cx1=1;tx=(xc+8)/16.0;} else {cx0=1;cx1=2;tx=(xc-8)/16.0;}
  if(yc<8){cy0=0;cy1=1;ty=(yc+8)/16.0;} else {cy0=1;cy1=2;ty=(yc-8)/16.0;}
  double a=g[cy0][cx0],b=g[cy0][cx1],c=g[cy1][cx0],d=g[cy1][cx1];
  double t=a+(b-a)*tx, bo=c+(d-c)*tx; return t+(bo-t)*ty;
}
void draw_natural_water(int sx,int sy, bool n,bool s,bool e,bool w,bool ne,bool nw,bool se,bool sw){
  const spr::Sprite& G=spr::TILE[0];              // herbe dessous (la rive laisse voir l'herbe)
  for(int j=0;j<16;j++)for(int i=0;i<16;i++) gb::pixel(sx+i,sy+j,G.px[j*16+i]);
  for(int y=0;y<16;y++)for(int x=0;x<16;x++){
    double c=cov8(x,y,n,s,e,w,ne,nw,se,sw);
    double th=0.5+0.17*vnoise(sx+x,sy+y);     // rive qui ONDULE sur plusieurs px -> plus de marche a 90 deg
    if(c<=th){ if(c>th-0.05 && (hash2(sx+x,sy+y)%100)<45) gb::pixel(sx+x,sy+y,gb::rgb(126,124,92)); // lisere humide intermittent
               continue; }
    double d=c-th; uint16_t col;
    if(d<=0.07)      col=gb::rgb(104,168,174);                               // haut-fond
    else if(d<=0.20) col=((x+y)&1)? gb::rgb(96,160,200):gb::rgb(78,140,194); // eau peu profonde
    else             col=((x+y)%6==0)? gb::rgb(96,152,214):gb::rgb(58,110,180); // eau profonde + rides
    gb::pixel(sx+x,sy+y,col);
  }
}

// ------- CHEMIN de terre a bords naturels (rive herbeuse ondulante) -------
// Meme principe que l'eau : la terre recule des bords ouverts (herbe) selon une
// ondulation fonction de la coordonnee absolue -> continue d'une tuile a l'autre,
// avec touffes d'herbe qui empietent et mottes de terre eparses pour un bord use.
static inline int hash2(int a,int b){ uint32_t u=(uint32_t)(a*374761393u + b*668265263u); u^=u>>13; u*=1274126177u; u^=u>>16; return (int)(u&0xFFFF); }
void draw_natural_path(int sx,int sy, bool n,bool s,bool e,bool w,bool ne,bool nw,bool se,bool sw){
  const spr::Sprite& G=spr::TILE[0];              // herbe dessous (bords herbeux)
  for(int j=0;j<16;j++)for(int i=0;i<16;i++) gb::pixel(sx+i,sy+j,G.px[j*16+i]);
  for(int y=0;y<16;y++)for(int x=0;x<16;x++){
    double c=cov8(x,y,n,s,e,w,ne,nw,se,sw);
    double th=0.5+0.17*vnoise(sx+x+71,sy+y+131);
    int h=hash2(sx+x,sy+y)%100;
    if(c<=th){ if(c>th-0.05 && h<22) gb::pixel(sx+x,sy+y,gb::rgb(150,120,74)); // mottes de terre au bord use
               continue; }
    double d=c-th; uint16_t col;
    if(d<=0.06)      col=gb::rgb(138,136,82);       // lisiere terre/herbe
    else if(d<=0.16) col=gb::rgb(186,152,98);       // terre tassee claire
    else             col=(h<14)? gb::rgb(201,174,124) : (h<30)? gb::rgb(150,116,66) : gb::rgb(172,136,82);
    if(d<=0.10 && (hash2(sx+x+3,sy+y+5)%100)<20) col=G.px[y*16+x];   // touffes d'herbe qui empietent
    gb::pixel(sx+x,sy+y,col);
  }
}

// ------- ROUTE pavee a bords naturels (meme recul herbeux ondulant que le chemin) -------
// Repond au retour "la route est trop droite" : pierres de taille irregulieres,
// bord qui recule selon shore_wob (continu d'une tuile a l'autre) + touffes d'herbe.
void draw_natural_road(int sx,int sy, bool n,bool s,bool e,bool w,bool ne,bool nw,bool se,bool sw){
  const spr::Sprite& G=spr::TILE[0];              // herbe dessous (bords herbeux)
  for(int j=0;j<16;j++)for(int i=0;i<16;i++) gb::pixel(sx+i,sy+j,G.px[j*16+i]);
  for(int y=0;y<16;y++)for(int x=0;x<16;x++){
    double c=cov8(x,y,n,s,e,w,ne,nw,se,sw);
    double th=0.5+0.15*vnoise(sx+x+211,sy+y+97);
    int h=hash2(sx+x,sy+y)%100;
    if(c<=th){ if(c>th-0.05 && h<16) gb::pixel(sx+x,sy+y,gb::rgb(120,120,130)); // paves qui debordent
               continue; }
    double d=c-th;
    bool joint = (((x+(hash2(sx,y)&1))&3)==0) || (((y+(hash2(x,sy)&1))&3)==0);
    uint16_t col;
    if(d<=0.05)    col=gb::rgb(110,110,120);        // lisiere pierre/herbe
    else if(joint) col=gb::rgb(120,120,130);        // joints sombres
    else           col=(h<16)? gb::rgb(176,176,184) : (h<30)? gb::rgb(132,132,142)
                      : (h<36)? gb::rgb(96,120,84) : gb::rgb(150,150,158);
    if(d<=0.10 && (hash2(sx+x+7,sy+y+2)%100)<18) col=G.px[y*16+x];  // touffes au bord
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

// ------- HERBE enrichie : tuile herbe + touffes/brins variant par cellule -------
void draw_grass(int sx,int sy,int gx,int gy){
  const spr::Sprite& G=spr::TILE[0];
  for(int y=0;y<16;y++)for(int x=0;x<16;x++) gb::pixel(sx+x,sy+y,G.px[y*16+x]);
  uint16_t bl_d=gb::rgb(52,96,50), bl_l=gb::rgb(106,172,98);  // brins sombres / clairs
  int n=2+(hash2(gx*5+1,gy*5+2)%3);                           // 2..4 touffes par case
  for(int k=0;k<n;k++){
    int bx=1+hash2(gx*13+k*3,gy*17+7)%14;
    int by=3+hash2(gx*11+k*5,gy*19+2)%11;
    uint16_t c=(hash2(bx+gx,by+gy)&1)? bl_l : bl_d;
    gb::pixel(sx+bx,sy+by,c); gb::pixel(sx+bx,sy+by-1,c);     // petit brin vertical
    if(k==0) gb::pixel(sx+bx+1,sy+by,c);                      // une touffe un peu plus large
  }
}

void tree_crown(int sx,int sy,int cx,double rad,int cy);   // defini juste apres draw_tree
// ------- ARBRE : houppier variant (taille/teinte/decalage selon la position) -------
void draw_tree(int sx,int sy){
  const spr::Sprite& G=spr::TILE[0];                       // herbe dessous
  for(int y=0;y<16;y++)for(int x=0;x<16;x++) gb::pixel(sx+x,sy+y,G.px[y*16+x]);
  int v=hash2(sx,sy);
  double rad = 6.6 + (double)(v%3)*0.5;                    // 6.6 .. 7.6 : couronne plus compacte -> deborde moins sur les cases voisines (sentiers visibles)
  int cx = 8 + (((v>>2)%3)-1);                             // leger decalage horizontal -1..+1
  tree_crown(sx,sy,cx,rad,-3);
}

// Houppier unique et coherent (une seule espece/palette) : reutilise par l'arbre et le bosquet.
// Silhouette feuillue (bord bosselé), lumiere en haut-gauche, ombre en bas-droite, grappes de feuilles.
void tree_crown(int sx,int sy,int cx,double rad,int cy){
  uint16_t trk=gb::rgb(100,68,36), trd=gb::rgb(72,48,26);
  for(int y=8;y<15;y++){ gb::pixel(sx+cx-1,sy+y,trd); gb::pixel(sx+cx,sy+y,trk); gb::pixel(sx+cx+1,sy+y,trd); }
  uint16_t md=gb::rgb(54,114,62), dk=gb::rgb(38,84,46), lt=gb::rgb(92,164,96), ds=gb::rgb(28,64,36);
  for(int yy=-14;yy<9;yy++)for(int xx=-3;xx<19;xx++){
    double nx=(xx-cx), ny=(yy-cy);
    // rayon bosselé selon l'angle -> bord feuillu plutot qu'ellipse lisse
    double ang=(ny!=0||nx!=0)? (nx*0.7+ny*1.1):0;
    double bump=0.85 + 0.15*((hash2((int)(ang*4)+37, (sx+sy)&31)%100)/100.0);
    double r=(nx*nx)/(rad*rad) + (ny*ny)/(11.0*11.0);
    if(r<=bump){
      // grappes de feuilles : bruit lisse par position absolue
      int hv=hash2(sx+xx, sy+yy)%100;
      uint16_t c=md;
      if(hv<24) c=dk;                                      // creux d'ombre entre feuilles
      // lumiere haut-gauche
      if(nx<-1.0 && ny<-1.0 && hv>30) c=lt;
      if(r>0.72 && hv<55) c=dk;                            // bord assombri
      if(ny>3.5) c=ds;                                     // dessous ombré
      gb::pixel(sx+xx,sy+yy,c);
    }
  }
}

// PONT en bois, pose SUR l'eau (elle arrive au ras). Planches perpendiculaires au
// sens de marche ; garde-corps seulement du cote de l'eau. w* = eau au voisin.
void draw_bridge(int sx,int sy, bool ww,bool we,bool wn,bool ws){
  uint16_t deck=gb::rgb(152,106,58), deckd=gb::rgb(116,78,40);
  uint16_t rail=gb::rgb(104,70,38), railhi=gb::rgb(152,108,60), post=gb::rgb(74,50,26);
  bool vert = (ww||we);                                    // eau sur les cotes -> traversee verticale
  for(int y=0;y<16;y++)for(int x=0;x<16;x++)
    gb::pixel(sx+x,sy+y, ((vert? (y&3):(x&3))==0)? deckd : deck);   // planches en travers
  if(ww){ for(int y=0;y<16;y++){ gb::pixel(sx+0,sy+y,post); gb::pixel(sx+1,sy+y,rail);} for(int y=0;y<16;y+=3) gb::pixel(sx+1,sy+y,railhi); }
  if(we){ for(int y=0;y<16;y++){ gb::pixel(sx+15,sy+y,post); gb::pixel(sx+14,sy+y,rail);} for(int y=0;y<16;y+=3) gb::pixel(sx+14,sy+y,railhi); }
  if(wn){ for(int x=0;x<16;x++){ gb::pixel(sx+x,sy+0,post); gb::pixel(sx+x,sy+1,rail);} for(int x=0;x<16;x+=3) gb::pixel(sx+x,sy+1,railhi); }
  if(ws){ for(int x=0;x<16;x++){ gb::pixel(sx+x,sy+15,post); gb::pixel(sx+x,sy+14,rail);} for(int x=0;x<16;x+=3) gb::pixel(sx+x,sy+14,railhi); }
}

// BOSQUET : grappe dense dans le MEME style que l'arbre (plus de sprite importe -> uniformite).
void draw_grove(int sx,int sy){
  const spr::Sprite& G=spr::TILE[0];
  for(int y=0;y<16;y++)for(int x=0;x<16;x++) gb::pixel(sx+x,sy+y,G.px[y*16+x]);
  tree_crown(sx,sy, 4, 6.2, -1);                           // petit arbre arriere-gauche
  tree_crown(sx,sy,12, 6.2, -1);                           // petit arbre arriere-droit
  tree_crown(sx,sy, 8, 7.6, -4);                           // arbre avant, un peu plus grand
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
  uint16_t fr=gb::rgb(70,44,24), post=gb::rgb(104,70,40), arch=gb::rgb(120,84,48);
  uint16_t leaf=gb::rgb(116,76,42), leafd=gb::rgb(92,60,32), seam=gb::rgb(64,42,22), leafHi=gb::rgb(150,104,60);
  uint16_t iron=gb::rgb(60,58,66), ironHi=gb::rgb(120,118,128), stud=gb::rgb(150,148,158), ring=gb::rgb(214,190,120);
  int x0=4,x1=11;
  // arc plein cintre : masque par ligne pour un haut arrondi
  auto span=[&](int y,int&a,int&b){ a=x0; b=x1;
    if(y==2){a=6;b=9;} else if(y==3){a=5;b=10;} else {a=x0;b=x1;} };
  // chambranle de pierre (montants + arc)
  for(int y=2;y<=15;y++){ int a,b; span(y,a,b); gb::pixel(sx+a-1,sy+y,post); gb::pixel(sx+b+1,sy+y,post); }
  for(int x=5;x<=10;x++) gb::pixel(sx+x,sy+1,arch);
  gb::pixel(sx+4,sy+2,arch); gb::pixel(sx+11,sy+2,arch);
  // vantail bois (planches verticales)
  for(int y=2;y<=15;y++){ int a,b; span(y,a,b);
    for(int x=a;x<=b;x++){ uint16_t c=leaf; if(((x-x0)%3)==0) c=leafd; gb::pixel(sx+x,sy+y,c); }
    gb::pixel(sx+a,sy+y,leafHi); }                          // reflet gauche
  for(int y=4;y<=15;y++){ gb::pixel(sx+7,sy+y,seam); }      // joint central
  // deux bandes de fer avec rivets
  for(int by=5; by<=11; by+=6){ for(int x=x0;x<=x1;x++){ gb::pixel(sx+x,sy+by,iron); gb::pixel(sx+x,sy+by-1,ironHi); }
    gb::pixel(sx+x0+1,sy+by,stud); gb::pixel(sx+x1-1,sy+by,stud); }
  // anneau de porte (laiton)
  gb::pixel(sx+9,sy+9,ring); gb::pixel(sx+10,sy+9,ring); gb::pixel(sx+9,sy+10,ring); gb::pixel(sx+10,sy+10,ring);
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

// ===================== MOBILIER DE TAVERNE (tuiles 23..27) =====================
// Chaque meuble pose d'abord le sol bois (tuile 5) puis l'objet par-dessus.
// Sol bois : MEMES planches que le sol ouvert (tuile 5 via draw_bridge, cas non-eau) -> plus de "sol tourne".
static void inn_floor(int sx,int sy){ uint16_t deck=gb::rgb(152,106,58), deckd=gb::rgb(116,78,40);
  for(int y=0;y<16;y++)for(int x=0;x<16;x++) gb::pixel(sx+x,sy+y, ((x&3)==0)? deckd:deck); }

void draw_counter(int sx,int sy){ inn_floor(sx,sy);                 // 23 comptoir
  gb::fill_rect(sx,sy+4,16,10,gb::rgb(96,62,32));
  gb::fill_rect(sx,sy+2,16,3, gb::rgb(150,104,60));                 // plateau
  gb::fill_rect(sx,sy+2,16,1, gb::rgb(188,142,88));                 // reflet
  gb::fill_rect(sx,sy+13,16,2,gb::rgb(52,34,18));                   // plinthe
  for(int x=sx+3;x<sx+16;x+=4) gb::fill_rect(x,sy+5,1,8,gb::rgb(70,46,24)); // planches
}
// 24 table : tuile multi-cellule. Rebord seulement sur les cotes SANS table voisine,
// pour que 2x2 tuiles forment une seule grande table de banquet.
void draw_table_tile(int sx,int sy,bool up,bool down,bool left,bool right){
  inn_floor(sx,sy);
  gb::Color top=gb::rgb(140,98,54), top2=gb::rgb(122,84,46), rim=gb::rgb(90,60,30), leg=gb::rgb(66,44,22), hi=gb::rgb(170,122,74);
  for(int y=1;y<15;y++)for(int x=1;x<15;x++) gb::pixel(sx+x,sy+y, ((y>>1)&1)?top2:top);   // plateau planches
  for(int y=2;y<15;y+=2)for(int x=1;x<15;x++) if((x+y)%4==0) gb::pixel(sx+x,sy+y,hi);       // veinage
  // vaisselle dressee : assiette (+ parfois chope / miche) -> table garnie
  { int hv=(int)(((sx*73856093)^(sy*19349663))&0xff);
    int ax=sx+3+(hv%7), ay=sy+5+((hv>>3)%5);
    gb::fill_rect(ax,ay,3,3,gb::rgb(226,226,214)); gb::pixel(ax+1,ay+1,gb::rgb(150,110,70)); // assiette + mets
    if(hv&1){ gb::fill_rect(ax+5,ay+1,2,3,gb::rgb(120,80,44)); gb::pixel(ax+7,ay+2,gb::rgb(120,80,44)); } // chope
    else    { gb::fill_rect(ax+5,ay+2,3,2,gb::rgb(196,150,86)); }                                        // miche de pain
  }
  if(!up)   for(int x=0;x<16;x++){ gb::pixel(sx+x,sy+0,rim); gb::pixel(sx+x,sy+1,rim); }
  if(!down) for(int x=0;x<16;x++){ gb::pixel(sx+x,sy+15,rim); gb::pixel(sx+x,sy+14,rim); }
  if(!left) for(int y=0;y<16;y++){ gb::pixel(sx+0,sy+y,rim); gb::pixel(sx+1,sy+y,rim); }
  if(!right)for(int y=0;y<16;y++){ gb::pixel(sx+15,sy+y,rim); gb::pixel(sx+14,sy+y,rim); }
  if(!down&&!left){  for(int k=0;k<3;k++) gb::pixel(sx+2,sy+15-k,leg); }                     // pieds visibles (avant)
  if(!down&&!right){ for(int k=0;k<3;k++) gb::pixel(sx+13,sy+15-k,leg); }
}
void draw_table(int sx,int sy){ draw_table_tile(sx,sy,false,false,false,false); }
void draw_barrel_obj(int sx,int sy){ inn_floor(sx,sy);             // 25 tonneau
  for(int y=2;y<15;y++){ int d=(y<8)?(y-2):(14-y); int bulge=d/3; int x0=3-bulge,x1=12+bulge;
    for(int x=x0;x<=x1;x++) gb::pixel(sx+x,sy+y, (x&1)?gb::rgb(108,72,38):gb::rgb(130,90,48)); }
  gb::fill_rect(sx+1,sy+4,14,1,gb::rgb(94,94,104)); gb::fill_rect(sx+1,sy+11,14,1,gb::rgb(94,94,104));
  gb::fill_rect(sx+5,sy+1,6,2,gb::rgb(92,60,32));                   // couvercle
}
void draw_bottles(int sx,int sy){ inn_floor(sx,sy);                // 26 etagere a bouteilles
  gb::fill_rect(sx,sy+3,16,2,gb::rgb(86,58,30)); gb::fill_rect(sx,sy+10,16,2,gb::rgb(86,58,30)); // 2 etageres
  const gb::Color col[3]={gb::rgb(60,120,66),gb::rgb(130,74,42),gb::rgb(150,52,52)};
  for(int i=0;i<4;i++){ int x=sx+2+i*4; gb::fill_rect(x,sy+5,2,5,col[i%3]); gb::pixel(x,sy+4,col[i%3]); }
  for(int i=0;i<4;i++){ int x=sx+3+i*4; gb::fill_rect(x,sy+12,2,3,col[(i+1)%3]); }
}
void draw_stool(int sx,int sy){ inn_floor(sx,sy);                  // 27 tabouret
  for(int y=0;y<16;y++)for(int x=0;x<16;x++){ int dx=x-8,dy=y-8; if(dx*dx+dy*dy<=13)
    gb::pixel(sx+x,sy+y, ((x+y)&1)?gb::rgb(104,68,36):gb::rgb(122,84,46)); }
  gb::pixel(sx+6,sy+6,gb::rgb(150,110,66));
}
// 30 cheminee : manteau de pierre, foyer sombre, buches et flammes
void draw_hearth(int sx,int sy){ inn_floor(sx,sy);
  uint16_t stone=gb::rgb(122,118,124), stoned=gb::rgb(84,80,88), dark=gb::rgb(28,22,22);
  uint16_t log=gb::rgb(98,62,32), logd=gb::rgb(70,44,22);
  uint16_t fy=gb::rgb(252,224,96), fo=gb::rgb(242,146,44), fr=gb::rgb(214,74,38);
  gb::fill_rect(sx,sy,16,16,stone);                                   // manteau
  for(int y=0;y<16;y+=4)for(int x=0;x<16;x++) gb::pixel(sx+x,sy+y,stoned);  // assises de pierre
  for(int x=0;x<16;x+=4)for(int y=0;y<16;y++) gb::pixel(sx+x,sy+y,stoned);
  gb::fill_rect(sx+2,sy+3,12,12,dark);                                // foyer
  gb::fill_rect(sx+3,sy+12,10,2,logd); gb::fill_rect(sx+4,sy+13,8,2,log);   // buches
  for(int x=0;x<10;x++){ int h=3+((x*7+5)%5); for(int k=0;k<h;k++){ uint16_t c=(k>=h-1)?fy:(k>=h-2)?fo:fr; gb::pixel(sx+3+x,sy+12-k,c);} } // flammes
  gb::pixel(sx+8,sy+6,fy);
}
// 31 tapis : natte rouge a bordure doree et losange creme (multi-tuiles : bordure seulement au bord)
void draw_rug(int sx,int sy,bool up,bool down,bool left,bool right){ inn_floor(sx,sy);
  uint16_t base=gb::rgb(150,42,44), base2=gb::rgb(126,32,36), bord=gb::rgb(200,170,90), cream=gb::rgb(222,202,152);
  for(int y=1;y<15;y++)for(int x=1;x<15;x++) gb::pixel(sx+x,sy+y, (((x+y)&3)==0)?base2:base);
  for(int y=0;y<16;y++)for(int x=0;x<16;x++){ int dx=x-8,dy=y-8; if(dx<0)dx=-dx; if(dy<0)dy=-dy; int d=dx+dy; if(d>=3&&d<=4) gb::pixel(sx+x,sy+y,cream); } // losange
  if(!up)   for(int x=0;x<16;x++){ gb::pixel(sx+x,sy+0,bord); }
  if(!down) for(int x=0;x<16;x++){ gb::pixel(sx+x,sy+15,bord); }
  if(!left) for(int y=0;y<16;y++){ gb::pixel(sx+0,sy+y,bord); }
  if(!right)for(int y=0;y<16;y++){ gb::pixel(sx+15,sy+y,bord); }
}
// 32 table ronde dressee (1 tuile = 1 table pour ~4 convives)
void draw_round_table(int sx,int sy){ inn_floor(sx,sy);
  uint16_t top=gb::rgb(140,98,54), top2=gb::rgb(122,84,46), rim=gb::rgb(88,58,30), hi=gb::rgb(172,124,76);
  for(int y=0;y<16;y++)for(int x=0;x<16;x++){ int dx=x-8,dy=y-8,d2=dx*dx+dy*dy;
    if(d2<=52){ uint16_t c=((y>>1)&1)?top2:top; if(d2>=42)c=rim; gb::pixel(sx+x,sy+y,c);} }
  for(int y=2;y<14;y+=2)for(int x=4;x<12;x++){ int dx=x-8,dy=y-8; if(dx*dx+dy*dy<=30 && (x+y)%4==0) gb::pixel(sx+x,sy+y,hi); }
  gb::fill_rect(sx+5,sy+5,3,3,gb::rgb(226,226,214)); gb::pixel(sx+6,sy+6,gb::rgb(150,110,70)); // assiette
  gb::fill_rect(sx+9,sy+8,2,2,gb::rgb(120,80,44));                                             // chope
}
void draw_inn_tile(int sx,int sy,int t){   // tuiles simples (la table 24 est geree a part, avec voisins)
  switch(t){ case 23:draw_counter(sx,sy);break;
             case 25:draw_barrel_obj(sx,sy);break; case 26:draw_bottles(sx,sy);break;
             default:draw_stool(sx,sy);break; }
}
}
