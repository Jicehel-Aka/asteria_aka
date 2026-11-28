// ASTERIA (Gamebuino AKA) - Etat de rasterisation commun a tous les ports gb::.
//
// Trois reglages, tous NEUTRES par defaut (dessin direct a l'ecran, comme avant) :
//   - cible   : set_target(buf,w,h) redirige pixel/fill_rect/draw_image* vers un
//               tampon RGB565 (nullptr = ecran). Sert au cache de carte (OPT-AKA-3)
//               et au tampon du donjon (OPT-AKA-1).
//   - origine : set_origin(ox,oy) soustrait (ox,oy) a toutes les coordonnees. Permet
//               de dessiner les tuiles en coordonnees CARTE (pixels absolus) : les
//               variations pseudo-aleatoires (hash des arbres...) restent stables
//               quand la camera defile.
//   - clip    : set_clip(x,y,w,h) restreint le dessin a un rectangle (coordonnees
//               cible, apres origine).
//
// Header-only (fonctions inline, etat unique en C++17) : aucun fichier a ajouter aux
// listes de compilation. Chaque port consulte raster_state() dans ses primitives.
#pragma once
#include <cstdint>
namespace gb {
struct RasterState {
  uint16_t* buf = nullptr; int bw = 0, bh = 0;      // cible hors ecran (nullptr = ecran)
  int ox = 0, oy = 0;                               // origine soustraite
  bool clip = false; int cx0 = 0, cy0 = 0, cx1 = 0, cy1 = 0;   // clip [cx0,cx1[ x [cy0,cy1[
  // Vrai si aucun reglage actif : les ports prennent alors leur chemin rapide d'origine.
  bool neutral() const { return !buf && !ox && !oy && !clip; }
};
inline RasterState& raster_state(){ static RasterState s; return s; }

inline void set_target(uint16_t* buf, int w, int h){ RasterState& r = raster_state(); r.buf = buf; r.bw = buf ? w : 0; r.bh = buf ? h : 0; }
inline void set_origin(int ox, int oy){ RasterState& r = raster_state(); r.ox = ox; r.oy = oy; }
inline void set_clip(int x, int y, int w, int h){ RasterState& r = raster_state(); r.clip = true; r.cx0 = x; r.cy0 = y; r.cx1 = x + w; r.cy1 = y + h; }
inline void reset_clip(){ raster_state().clip = false; }
inline void reset_raster(){ RasterState& r = raster_state(); r = RasterState(); }

// Applique origine + clip ; renvoie false si le point est rejete.
// Les bornes de la cible (tampon ou ecran sw x sh) sont aussi verifiees ici.
inline bool raster_map(int& x, int& y, int sw, int sh){
  const RasterState& r = raster_state();
  x -= r.ox; y -= r.oy;
  if (r.clip && (x < r.cx0 || y < r.cy0 || x >= r.cx1 || y >= r.cy1)) return false;
  int w = r.buf ? r.bw : sw, h = r.buf ? r.bh : sh;
  return (unsigned)x < (unsigned)w && (unsigned)y < (unsigned)h;
}
}
