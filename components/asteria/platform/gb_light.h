// ASTERIA (Gamebuino AKA) - Eclairage jour / nuit applique a l'image deja dessinee.
//
// light_row() assombrit une rangee de pixels RGB565 (R bits 0-4, G 5-10, B 11-15,
// comme gb::rgb) selon une lumiere ambiante (0..256) et une petite liste de sources
// (heros, portes eclairees, foyers...). La nuit est bleutee (le bleu s'assombrit
// moins), les sources sont chaudes (orangees). Le calcul est fait en entiers.
//
// Header-only : le meme code tourne sur PC (framebuffer en memoire) et sur l'AKA
// (rangee lue/ecrite via lcd_getpixel/lcd_putpixel), voir gb::light_screen().
#pragma once
#include <cstdint>
namespace gb {

struct Light { int16_t x, y, r; uint8_t warm; };   // centre ecran, rayon (px), chaleur 0..255
enum { LIGHT_MAX = 24 };

inline int light_isqrt(int v){ int r = 0; while ((r + 1) * (r + 1) <= v) ++r; return r; }

// px : rangee de w pixels a l'ordonnee y ; amb : 0 (noir) .. 256 (aucun effet)
inline void light_row(uint16_t* px, int w, int y, int amb, const Light* L, int n){
  static uint16_t f[320]; static uint8_t wm[320];
  if (w > 320) w = 320;
  for (int x = 0; x < w; ++x) { f[x] = (uint16_t)amb; wm[x] = 0; }
  for (int i = 0; i < n; ++i) {
    const Light& l = L[i];
    int dy = y - l.y; if (dy <= -l.r || dy >= l.r) continue;
    int r2 = l.r * l.r, half = light_isqrt(r2 - dy * dy);
    int x0 = l.x - half, x1 = l.x + half; if (x0 < 0) x0 = 0; if (x1 >= w) x1 = w - 1;
    for (int x = x0; x <= x1; ++x) {
      int dx = x - l.x, d2 = dx * dx + dy * dy; if (d2 >= r2) continue;
      int k = 256 - (d2 << 8) / r2;                          // 256 au centre -> 0 au bord
      k = (k * k) >> 8;                                      // bord adouci
      int lit = amb + (((256 - amb) * k) >> 8);
      if (lit > f[x]) f[x] = (uint16_t)lit;
      int wv = (k * l.warm) >> 8; if (wv > wm[x]) wm[x] = (uint8_t)wv;
    }
  }
  for (int x = 0; x < w; ++x) {
    int fx = f[x], wv = wm[x];
    if (fx >= 256 && wv == 0) continue;
    uint16_t c = px[x];
    int r = c & 31, g = (c >> 5) & 63, b = c >> 11;
    // Nuit bleutee : plus un pixel est sombre (d grand), plus le rouge et le vert baissent
    // par rapport au bleu. Les sources de lumiere ajoutent une teinte chaude (orangee).
    int d = 256 - fx;
    int rf = fx - (d >> 3) + (wv >> 2), gf = fx - (d >> 4) + (wv >> 4), bf = fx + (d >> 2) - (wv >> 3);
    if (rf < 0) rf = 0; if (gf < 0) gf = 0; if (bf < 0) bf = 0;
    r = (r * rf) >> 8; g = (g * gf) >> 8; b = (b * bf) >> 8;
    if (r > 31) r = 31; if (g > 63) g = 63; if (b > 31) b = 31;
    px[x] = (uint16_t)(r | (g << 5) | (b << 11));
  }
}

}
