#!/usr/bin/env python3
# ASTERIA - Heros masculin (le Voyageur) : decoupe tools/icon_src/voyageur_planche.png
# (fond magenta, 4 rangees x 3 frames : bas, haut, gauche, et une 4e rangee non utilisee
# car elle marche aussi vers la gauche ; la droite est le miroir de la gauche) et genere
# components/asteria/generated/asteria_hero_m.h : spr::HERO_M[4][3], meme format que
# spr::HERO (hauteur 20 px, cle magenta). Detourage et reduction : voir bake_icons.py.
# Relancer : python3 tools/bake_hero_m.py   (depuis la racine du depot)
import sys, os
sys.path.insert(0, os.path.dirname(__file__))
from PIL import Image, ImageFilter
from bake_icons import key_out, to_565

SRC = "tools/icon_src/voyageur_planche.png"
OUT = "components/asteria/generated/asteria_hero_m.h"
H = 20   # hauteur des frames en jeu (comme spr::HERO)

def components(alpha):
    """Boites englobantes des silhouettes (alpha > 0), petites taches ignorees."""
    a = alpha.point(lambda v: 255 if v > 64 else 0).filter(ImageFilter.MaxFilter(9))
    w, h = a.size; px = a.load(); seen = bytearray(w * h); boxes = []
    for y in range(0, h, 2):
        for x in range(0, w, 2):
            if px[x, y] == 0 or seen[y * w + x]: continue
            stack = [(x, y)]; seen[y * w + x] = 1; x0 = x1 = x; y0 = y1 = y; n = 0
            while stack:
                cx, cy = stack.pop(); n += 1
                x0, x1, y0, y1 = min(x0, cx), max(x1, cx), min(y0, cy), max(y1, cy)
                for nx, ny in ((cx + 1, cy), (cx - 1, cy), (cx, cy + 1), (cx, cy - 1)):
                    if 0 <= nx < w and 0 <= ny < h and px[nx, ny] and not seen[ny * w + nx]:
                        seen[ny * w + nx] = 1; stack.append((nx, ny))
            if n > 2000: boxes.append((x0, y0, x1 + 1, y1 + 1))
    return boxes

def shrink(im, h):
    """Reduction premultipliee a la hauteur h, alpha binarise (sprite a cle de couleur)."""
    w = max(1, round(im.width * h / im.height))
    px = im.load(); pm = Image.new("RGBA", im.size); pp = pm.load()
    for y in range(im.height):
        for x in range(im.width):
            r, g, b, a = px[x, y]; pp[x, y] = (r * a // 255, g * a // 255, b * a // 255, a)
    pm = pm.resize((w, h), Image.LANCZOS)
    out = Image.new("RGBA", (w, h)); op = out.load(); sp = pm.load()
    for y in range(h):
        for x in range(w):
            r, g, b, a = sp[x, y]
            op[x, y] = (0, 0, 0, 0) if a < 110 else (min(255, r * 255 // a), min(255, g * 255 // a), min(255, b * 255 // a), 255)
    return out

def main():
    sheet = key_out(Image.open(SRC).convert("RGB"))
    boxes = components(sheet.getchannel("A"))
    boxes.sort(key=lambda b: (b[1] + b[3]) // 2)
    rows = []
    for b in boxes:                                   # regroupe par rangee (centre vertical)
        cy = (b[1] + b[3]) // 2
        if rows and abs(rows[-1][0] - cy) < 80: rows[-1][1].append(b)
        else: rows.append([cy, [b]])
    rows = [sorted(r[1], key=lambda b: b[0]) for r in rows]
    assert len(rows) >= 3 and all(len(r) == 3 for r in rows[:3]), [len(r) for r in rows]
    frames = [[shrink(sheet.crop(b), H) for b in r] for r in rows[:3]]
    frames.append([f.transpose(Image.FLIP_LEFT_RIGHT) for f in frames[2]])   # droite = miroir de gauche
    L = ["// ASTERIA - Heros masculin (genere par tools/bake_hero_m.py, ne pas editer).",
         "// HERO_M[dir][frame] : dir 0 bas, 1 haut, 2 gauche, 3 droite ; meme format que spr::HERO.",
         "#pragma once", "#include <cstdint>", "namespace asteria { namespace spr {",
         "#ifndef ASTERIA_SPR_DEFS", "#define ASTERIA_SPR_DEFS",
         "struct Sprite { const uint16_t* px; int16_t w; int16_t h; };",
         "static const uint16_t KEY = 0xF81F;", "#endif"]
    tab = []
    for d, row in enumerate(frames):
        cells = []
        for f, im in enumerate(row):
            v = to_565(im); name = f"HERO_M_{d}_{f}"
            L.append(f"static const uint16_t {name}[] = {{" + ",".join(f"0x{x:04X}" for x in v) + "};")
            cells.append(f"{{{name},{im.width},{im.height}}}")
        tab.append("{" + ",".join(cells) + "}")
    L.append("static const Sprite HERO_M[4][3] = {" + ",".join(tab) + "};")
    L.append("}}")
    open(OUT, "w").write("\n".join(L) + "\n")
    print(OUT, [[im.size for im in r] for r in frames])

if __name__ == "__main__":
    main()
