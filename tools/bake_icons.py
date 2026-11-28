#!/usr/bin/env python3
# ASTERIA - Decoupe les 5 planches d'icones d'objets (tools/icon_src/plancheN.png, fond
# magenta, grille 3x3 ou 3x2, voir docs/prompts_icones_objets.md) et genere
# components/asteria/generated/asteria_icons.h :
#   ICON24[i] (sac, boutique, forge) et ICON48[i] (fenetre "objet obtenu"),
#   i = index dans data::ITEMS (OBJ_0001 -> 0) ; ICON_GOLD = bourse d'or (index ITEM_COUNT).
# Pixels BGR565 (R bits 0-4, comme gb::rgb), cle magenta 0xF81F comme les autres sprites.
#
# Detourage : le fond genere n'est jamais un magenta pur (236,9,240...). La "force magenta"
# m = min(R,B) - G donne une transparence douce ; la frange est decontaminee (on retire
# la part de magenta), puis la reduction se fait en alpha premultiplie (pas de halo rose).
# Relancer : python3 tools/bake_icons.py   (depuis la racine du depot)
import os
from PIL import Image, ImageFilter

SRC = "tools/icon_src"
OUT = "components/asteria/generated/asteria_icons.h"
PREVIEW = os.environ.get("ICON_PREVIEW")   # chemin PNG optionnel : planche de controle

# Ordre des cases (gauche -> droite, haut -> bas) -> ID objet ; None = bourse d'or
SHEETS = [
    ("planche1.png", 3, 3, [37, 1, 32, 2, 8, 20, 27, 23, 10]),
    ("planche2.png", 3, 3, [5, 33, 34, 35, 26, 36, 12, 13, 31]),
    ("planche3.png", 3, 3, [3, 11, 19, 4, 25, 9, 21, 18, 24]),
    ("planche4.png", 3, 3, [6, 16, 17, 22, 41, 39, 7, 15, 38]),
    ("planche5.png", 3, 2, [40, 14, 28, 29, 30, None]),
    ("planche6.png", 3, 2, [42, 43, 44, 45, 46, None]),   # v1.5, optionnelle (case 6 libre)
]
import re as _re
# nombre d'objets lu dans le jeu : les objets sans icone ont une case vide (ICON_HAS = 0)
ITEM_COUNT = int(_re.search(r"ITEM_COUNT=(\d+)", open("components/asteria/generated/asteria_gen.h", encoding="utf-8").read()).group(1))
SIZES = (24, 48)

def bgr565(r, g, b): return ((b >> 3) << 11) | ((g >> 2) << 5) | (r >> 3)
KEY = 0xF81F

def key_out(cell):
    """RGB -> RGBA : alpha depuis la force magenta, couleur decontaminee."""
    rgb = cell.convert("RGB"); w, h = rgb.size
    src = rgb.load(); out = Image.new("RGBA", (w, h)); dst = out.load()
    for y in range(h):
        for x in range(w):
            r, g, b = src[x, y]
            m = min(r, b) - g                       # ~230 sur le fond, <80 sur les objets
            a = 255 if m < 90 else 0 if m > 170 else 255 * (170 - m) // 80
            if 0 < a < 255:                         # retire la part de fond (255,0,255)
                k = 255 - a
                r = max(0, min(255, (r * 255 - k * 255) // a)); b = max(0, min(255, (b * 255 - k * 255) // a))
                g = min(255, g * 255 // a)
            dst[x, y] = (r, g, b, a)
    return out

def crop_alpha(im):
    a = im.getchannel("A").point(lambda v: 255 if v > 64 else 0)
    a = a.filter(ImageFilter.MinFilter(3)).filter(ImageFilter.MaxFilter(3))   # ignore les poussieres isolees
    bb = a.getbbox()
    return im.crop(bb) if bb else im

def fit(im, size):
    """Reduction premultipliee dans un carre size x size (marge 1 px), centree."""
    inner = size - 2
    s = inner / max(im.width, im.height)
    w, h = max(1, round(im.width * s)), max(1, round(im.height * s))
    px = im.load(); pm = Image.new("RGBA", im.size); pp = pm.load()
    for y in range(im.height):
        for x in range(im.width):
            R, G, B, A = px[x, y]; pp[x, y] = (R * A // 255, G * A // 255, B * A // 255, A)
    pm = pm.resize((w, h), Image.LANCZOS)
    if size <= 24: pm = pm.filter(ImageFilter.UnsharpMask(radius=1, percent=60, threshold=2))
    canvas = Image.new("RGBA", (size, size), (0, 0, 0, 0))
    canvas.paste(pm, ((size - w) // 2, (size - h) // 2))
    return canvas

def to_565(im):
    px = im.load(); vals = []
    for y in range(im.height):
        for x in range(im.width):
            r, g, b, a = px[x, y]
            if a < 110: vals.append(KEY); continue
            r, g, b = (min(255, c * 255 // a) for c in (r, g, b))
            v = bgr565(r, g, b)
            vals.append(v ^ 0x0020 if v == KEY else v)   # un vrai magenta ne doit pas devenir transparent
    return vals

def main():
    icons = {}   # slot -> {size: RGBA}
    for fn, cols, rows, ids in SHEETS:
        if not os.path.exists(os.path.join(SRC, fn)): continue   # planche pas encore fournie
        sheet = Image.open(os.path.join(SRC, fn)).convert("RGB")
        cw, ch = sheet.width / cols, sheet.height / rows
        for i, oid in enumerate(ids):
            if oid is None and fn != "planche5.png": continue
            cx, cy = i % cols, i // cols
            cell = sheet.crop((round(cx * cw), round(cy * ch), round((cx + 1) * cw), round((cy + 1) * ch)))
            obj = crop_alpha(key_out(cell))
            slot = ITEM_COUNT if oid is None else oid - 1
            icons[slot] = {s: fit(obj, s) for s in SIZES}
    missing = [i + 1 for i in range(ITEM_COUNT) if i not in icons]
    if missing: print("sans icone (case provisoire en jeu) : " + ", ".join(f"OBJ_{m:04d}" for m in missing))
    blank = {sz: Image.new("RGBA", (sz, sz), (0, 0, 0, 0)) for sz in SIZES}
    for i in range(ITEM_COUNT): icons.setdefault(i, blank)

    L = ["// ASTERIA - Icones d'objets (genere par tools/bake_icons.py, ne pas editer).",
         "// ICON24 / ICON48 : index = rang dans data::ITEMS (OBJ_0001 -> 0) ; ICON_GOLD = bourse d'or.",
         "#pragma once", "#include <cstdint>", "namespace asteria { namespace icons {",
         f"static const int ICON_COUNT = {ITEM_COUNT + 1};", f"static const int ICON_GOLD = {ITEM_COUNT};",
         f"static const uint16_t KEY = 0x{KEY:04X};",
         "static const uint8_t ICON_HAS[ICON_COUNT] = {" + ",".join("0" if (i + 1) in missing else "1" for i in range(ITEM_COUNT)) + ",1};"]
    for s in SIZES:
        L.append(f"static const uint16_t ICON{s}[ICON_COUNT][{s * s}] = {{")
        for slot in range(ITEM_COUNT + 1):
            v = to_565(icons[slot][s])
            tag = "bourse d'or" if slot == ITEM_COUNT else f"OBJ_{slot + 1:04d}"
            body = ",".join(f"0x{x:04X}" for x in v)
            L.append(f"  {{ /* {tag} */ {body} }},")
        L.append("};")
    # Poupee d'equipement facultative (tools/icon_src/poupee.png, fond magenta) : silhouette
    # de face affichee derriere les cases de la fenetre personnage, au plus 100 x 116 px.
    dp = os.path.join(SRC, "poupee.png")
    if os.path.exists(dp):
        doll = crop_alpha(key_out(Image.open(dp).convert("RGB")))
        sc = min(100 / doll.width, 116 / doll.height)
        dw, dh = max(1, round(doll.width * sc)), max(1, round(doll.height * sc))
        canvas = Image.new("RGBA", (dw + 2, dh + 2), (0, 0, 0, 0))
        tmp = fit(doll, max(dw, dh) + 2)      # reduction premultipliee
        canvas = tmp.crop(((tmp.width - dw) // 2 - 1, (tmp.height - dh) // 2 - 1, (tmp.width + dw) // 2 + 1, (tmp.height + dh) // 2 + 1))
        v = to_565(canvas)
        L.append(f"static const int DOLL_W = {canvas.width}, DOLL_H = {canvas.height};")
        L.append("static const uint16_t DOLL[] = {" + ",".join(f"0x{x:04X}" for x in v) + "};")
        print(f"poupee : {canvas.width}x{canvas.height}")
    else:
        L.append("static const int DOLL_W = 0, DOLL_H = 0;")
        L.append("static const uint16_t DOLL[1] = {0};")
    # Portraits des PNJ (tools/icon_src/portraits_pnj.png, 3x3, fond magenta) : 48x48,
    # personnage pose en bas de la case. PORTRAIT_NPC = index de dialogue (dlg::DIALOG).
    pp = os.path.join(SRC, "portraits_pnj.png")
    NPCS = [21, 0, 5, 15, 16, 19, 18, 8, 1]   # Voyageur, Aldren, Lysa, Garrick, Tomas Venn, Meluve, Racha, Agnes, Doran
    if os.path.exists(pp):
        sheet = Image.open(pp).convert("RGB"); cw, ch = sheet.width / 3, sheet.height / 3
        L.append(f"static const int PORTRAIT_COUNT = {len(NPCS)};")
        L.append("static const int8_t PORTRAIT_NPC[] = {" + ",".join(map(str, NPCS)) + "};")
        L.append("static const uint16_t PORTRAIT[][48 * 48] = {")
        for i in range(len(NPCS)):
            cx, cy = i % 3, i // 3
            obj = crop_alpha(key_out(sheet.crop((round(cx * cw), round(cy * ch), round((cx + 1) * cw), round((cy + 1) * ch)))))
            f = fit(obj, 50)                                   # 48 utiles + marge de fit()
            canvas = Image.new("RGBA", (48, 48), (0, 0, 0, 0))
            bb = f.getbbox() or (0, 0, 48, 48); f = f.crop(bb)
            canvas.paste(f, ((48 - f.width) // 2, 48 - f.height), f)   # pose en bas
            L.append("  {" + ",".join(f"0x{x:04X}" for x in to_565(canvas)) + "},")
        L.append("};")
        print("portraits :", len(NPCS))
    else:
        L.append("static const int PORTRAIT_COUNT = 0;")
        L.append("static const int8_t PORTRAIT_NPC[1] = {-1};")
        L.append("static const uint16_t PORTRAIT[1][48 * 48] = {{0}};")
    L.append("}}")
    open(OUT, "w").write("\n".join(L) + "\n")
    print(f"{OUT} : {ITEM_COUNT + 1} icones x {SIZES}")

    if PREVIEW:   # controle visuel : toutes les icones sur fond sombre, en 48 puis 24 (x2)
        n = ITEM_COUNT + 1; per = 14
        pv = Image.new("RGB", (per * 56, ((n + per - 1) // per) * 2 * 56), (40, 34, 30))
        for slot in range(n):
            x, y = (slot % per) * 56 + 4, (slot // per) * 112 + 4
            pv.paste(icons[slot][48], (x, y), icons[slot][48])
            big = icons[slot][24].resize((48, 48), Image.NEAREST)
            pv.paste(big, (x, y + 56), big)
        pv.save(PREVIEW)

if __name__ == "__main__":
    main()
