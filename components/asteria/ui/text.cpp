// ASTERIA (Gamebuino AKA) - Rendu de texte bitmap (police CP1252 + decodage UTF-8) et mesure text_w().
#include "ui/text.h"
#include "generated/font_asteria_body.h"
#include "generated/font_asteria.h"
#include "generated/text_style_gold.h"
namespace asteria { namespace ui {
// Lit le prochain point de code UTF-8 et le mappe vers CP1252 (0x20..0xFF).
static inline uint8_t next_cp(const unsigned char*& p){
  unsigned char c = *p++;
  if(c < 0x80) return c;
  // Octet de tete suivi d'un octet hors 0x80..0xBF : ce n'est pas de l'UTF-8 mais
  // un caractere CP1252 isole (ex. texte charge depuis la SD) -> affiche tel quel,
  // sans consommer le caractere suivant.
  if((c == 0xC2 || c == 0xC3) && (*p & 0xC0) == 0x80){
    unsigned char d = *p++;
    return (uint8_t)(((c & 0x03) << 6) | (d & 0x3F));       // U+0080..U+00FF = CP1252/Latin-1
  }
  if(c == 0xC5 && (p[0] == 0x93 || p[0] == 0x92)){
    unsigned char d = *p++;
    return (d == 0x93) ? 0x9C : 0x8C;                        // oe / OE
  }
  if(c == 0xE2 && p[0] == 0x80 && p[1]){
    unsigned char d = p[1]; p += 2;
    if(d == 0x98 || d == 0x99) return '\'';
    if(d == 0x9C || d == 0x9D) return '"';
    if(d == 0x93 || d == 0x94) return '-';
    return '?';
  }
  while((*p & 0xC0) == 0x80) ++p;
  return (c >= 0x20) ? c : (uint8_t)'?';
}
static inline uint8_t idx_body(uint8_t cp){ return (cp < FONTB_FIRST) ? (uint8_t)('?' - FONTB_FIRST) : (uint8_t)(cp - FONTB_FIRST); }
static inline uint8_t idx_big (uint8_t cp){ return (cp < FONT_FIRST)  ? (uint8_t)('?' - FONT_FIRST)  : (uint8_t)(cp - FONT_FIRST); }
void text(int x,int y,const char* s, gb::Color c){
  if(!s) return;
  for(const unsigned char* p=(const unsigned char*)s; *p; ){
    uint8_t g=idx_body(next_cp(p));
    for(int r=0;r<FONTB_HEIGHT;++r){ uint16_t row=FONTB_BITS[g][r];
      for(int col=0;col<16;++col) if(row&(1<<(15-col))) gb::pixel(x+col,y+r,c); }
    x+=FONTB_ADV[g]+1;
  }
}
void text_big(int x,int y,const char* s, gb::Color c){
  if(!s) return;
  for(const unsigned char* p=(const unsigned char*)s; *p; ){
    uint8_t g=idx_big(next_cp(p));
    for(int r=0;r<FONT_HEIGHT;++r){ uint16_t row=FONT_BITS[g][r];
      for(int col=0;col<16;++col) if(row&(1<<(15-col))) gb::pixel(x+col,y+r,c); }
    x+=FONT_ADV[g]+1;
  }
}
int text_w(const char* s){
  if(!s) return 0;
  int w=0; for(const unsigned char* p=(const unsigned char*)s;*p;) w+=FONTB_ADV[idx_body(next_cp(p))]+1;
  return w;
}
// dégradé or (par ligne) + contour 4-voisins ; bright=false -> atténué
void gold(int x,int y,const char* s, bool bright){
  if(!s) return;
  gb::Color oc=gb::rgb(GOLD_OUTLINE.r,GOLD_OUTLINE.g,GOLD_OUTLINE.b);
  int x0=x;
  for(const unsigned char* p=(const unsigned char*)s; *p; ){ uint8_t g=idx_body(next_cp(p));
    for(int r=0;r<FONTB_HEIGHT;++r){ uint16_t row=FONTB_BITS[g][r];
      for(int col=0;col<16;++col) if(row&(1<<(15-col))){
        int px=x+col,py=y+r;
        gb::pixel(px-1,py,oc); gb::pixel(px+1,py,oc); gb::pixel(px,py-1,oc); gb::pixel(px,py+1,oc);
      }}
    x+=FONTB_ADV[g]+1; }
  // passe 2 : remplissage dégradé
  x=x0;
  for(const unsigned char* p=(const unsigned char*)s; *p; ){ uint8_t g=idx_body(next_cp(p));
    for(int r=0;r<FONTB_HEIGHT;++r){ uint16_t row=FONTB_BITS[g][r];
      const RGB& gc=GOLD_GRAD[r];
      uint8_t rr=gc.r,gg=gc.g,bb=gc.b;
      if(!bright){ rr=(uint8_t)(rr*0.72f); gg=(uint8_t)(gg*0.72f); bb=(uint8_t)(bb*0.72f); }
      gb::Color c=gb::rgb(rr,gg,bb);
      for(int col=0;col<16;++col) if(row&(1<<(15-col))) gb::pixel(x+col,y+r,c);
    }
    x+=FONTB_ADV[g]+1; }
}
}}
