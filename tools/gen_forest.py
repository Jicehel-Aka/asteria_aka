#!/usr/bin/env python3
# Forêt de Brume (MAP_0002, 20x15) — version organique :
#  - sentier sinueux reliant les 2 portails (bas (10,14) -> haut (10,0))
#  - ruisseau naturel (eau_nat 16, bords doux via autotile) qui serpente,
#    franchi par un petit pont (5) là où le sentier le croise
#  - arbres (6) en bosquets de densité variable, avec clairières
# Tuiles : 0 herbe, 1 chemin, 5 pont, 6 arbre, 16 eau_nat
import re, math, os
W, H = 20, 15
HERBE, CHEMIN, PONT, ARBRE, EAU = 0, 1, 5, 6, 16

g = [[HERBE for _ in range(W)] for _ in range(H)]

# --- hash pseudo-aléatoire déterministe (pas de dépendance) ------------------
def rnd(x, y, s=0):
    h = (x*73856093) ^ (y*19349663) ^ (s*83492791)
    h &= 0xFFFFFFFF
    h ^= h >> 13; h = (h*1274126177) & 0xFFFFFFFF; h ^= h >> 16
    return (h & 0xFFFF) / 65535.0

# --- 1) sentier sinueux : x varie autour de 10 en montant -------------------
path_cells = set()
def carve_path():
    y = 14
    while y >= 0:
        # décalage sinueux, mais recalé à x=10 aux deux extrémités (portails)
        if y >= 13 or y <= 1:
            cx = 10
        else:
            cx = int(round(10 + 2.6*math.sin((14-y)*0.72) + 1.4*math.sin((14-y)*0.27)))
            cx = max(3, min(W-4, cx))
        for dx in (0,): path_cells.add((cx, y))
        # largeur 2 par endroits pour un sentier plus naturel
        if rnd(cx, y, 7) > 0.55: path_cells.add((min(W-2, cx+1), y))
        y -= 1
    # relie les sauts horizontaux (quand cx change de +-2 d'une ligne à l'autre)
    pts = {}
    for (x, yy) in path_cells: pts.setdefault(yy, []).append(x)
carve_path()

# comble les trous horizontaux entre deux lignes successives du sentier
col_by_row = {}
for (x, y) in list(path_cells):
    col_by_row.setdefault(y, []).append(x)
for y in range(14, 0, -1):
    if y in col_by_row and (y-1) in col_by_row:
        a = int(round(sum(col_by_row[y])/len(col_by_row[y])))
        b = int(round(sum(col_by_row[y-1])/len(col_by_row[y-1])))
        lo, hi = sorted((a, b))
        for x in range(lo, hi+1):
            path_cells.add((x, y))

# --- 2) ruisseau serpentant (horizontal, autour de y=5), continu ------------
river_cells = set()
prev = None
for x in range(W):
    ry = int(round(5 + 1.8*math.sin(x*0.55) + 1.1*math.sin(x*0.23+1)))
    ry = max(2, min(H-3, ry))
    river_cells.add((x, ry))
    if rnd(x, ry, 3) > 0.5: river_cells.add((x, min(H-3, ry+1)))   # largeur 1-2
    # relie à la colonne précédente pour un vrai cours d'eau continu
    if prev is not None:
        lo, hi = sorted((prev, ry))
        for yy in range(lo, hi+1):
            river_cells.add((x, yy))
    prev = ry

# --- 3) pose : eau d'abord, puis pont aux croisements, puis sentier ---------
for (x, y) in river_cells:
    g[y][x] = EAU
for (x, y) in path_cells:
    if g[y][x] == EAU:
        g[y][x] = PONT        # le sentier franchit le ruisseau
    else:
        g[y][x] = CHEMIN

# garantit les portails praticables
for (px, py) in [(10, 14), (10, 0), (10, 13)]:
    if g[py][px] == EAU: g[py][px] = PONT
    elif g[py][px] != PONT: g[py][px] = CHEMIN

# --- 4) arbres en bosquets, avec clairières ---------------------------------
def near(cells, x, y, r=1):
    for dy in range(-r, r+1):
        for dx in range(-r, r+1):
            if (x+dx, y+dy) in cells: return True
    return False

for y in range(H):
    for x in range(W):
        if g[y][x] != HERBE: continue
        if (x, y) == (6, 10): continue                    # case du PNJ (npc 9)
        if near(path_cells, x, y, 1): continue            # dégage les abords du sentier
        if near(river_cells, x, y, 0): continue
        # densité par bosquets : bruit basse fréquence + bord plus boisé
        cluster = 0.5*rnd(x//3, y//3, 11) + 0.5*rnd(x//2, y//2, 5)
        edge = 0.22 if (x < 2 or x > W-3 or y < 1 or y > H-2) else 0.0
        thr = 0.52 - edge
        if cluster > thr and rnd(x, y, 9) > 0.30:
            g[y][x] = ARBRE

# clairière autour du PNJ et du départ
for (cx, cy, r) in [(6, 10, 1), (10, 13, 1)]:
    for dy in range(-r, r+1):
        for dx in range(-r, r+1):
            x, y = cx+dx, cy+dy
            if 0 <= x < W and 0 <= y < H and g[y][x] == ARBRE:
                g[y][x] = HERBE

# --- 5) sérialise et réécrit MAP1_T dans asteria_maps.h ---------------------
flat = [g[y][x] for y in range(H) for x in range(W)]
arr = ", ".join(str(v) for v in flat)
newline = "static const uint8_t MAP1_T[]={" + arr + "};"

here = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
hp = os.path.join(here, "components", "asteria", "generated", "asteria_maps.h")
src = open(hp).read()
src2 = re.sub(r"static const uint8_t MAP1_T\[\]=\{[^}]*\};", newline, src, count=1)
assert src2 != src, "MAP1_T introuvable"
open(hp, "w").write(src2)

trees = sum(1 for v in flat if v == ARBRE)
water = sum(1 for v in flat if v == EAU)
pont  = sum(1 for v in flat if v == PONT)
print(f"Forêt v2 : {trees} arbres, {water} eau, {pont} pont(s), sentier {len(path_cells)} cases")
