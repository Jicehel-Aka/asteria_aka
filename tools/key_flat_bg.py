# ASTERIA - Detourage d une planche generee sur fond uni NON magenta (ex. gris clair, quand le
# generateur ignore la consigne) : remplissage depuis les bords + nettoyage de la frange claire.
# Usage : python3 tools/key_flat_bg.py planche.png dossier_sortie  -> portraits_pnj_magenta.png
# (fond magenta, a copier dans tools/icon_src/) + apercus 48x48. Puis : python3 tools/bake_icons.py
import sys
from collections import deque
from PIL import Image
src, out = sys.argv[1], sys.argv[2]
im = Image.open(src).convert("RGB"); W, H = im.size; px = im.load()
bg = px[3, 3]
def near(c, tol=26): return all(abs(a - b) <= tol for a, b in zip(c, bg))
mask = bytearray(W * H)            # 1 = fond
q = deque((x, y) for x in range(W) for y in (0, H - 1)); q.extend((x, y) for y in range(H) for x in (0, W - 1))
while q:
    x, y = q.popleft()
    if mask[y * W + x] or not near(px[x, y]): continue
    mask[y * W + x] = 1
    for nx, ny in ((x+1,y),(x-1,y),(x,y+1),(x,y-1)):
        if 0 <= nx < W and 0 <= ny < H and not mask[ny * W + nx]: q.append((nx, ny))
# Poches de fond enfermees (entre l'arc et le corps, entre des meches...) : composantes de
# pixels quasi identiques au fond (tolerance serree, pour epargner cheveux gris et hermine)
seen = bytearray(W * H)
for sy in range(H):
    for sx in range(W):
        k = sy * W + sx
        if mask[k] or seen[k] or not near(px[sx, sy], 10): continue
        comp = []; q2 = deque([(sx, sy)]); seen[k] = 1
        while q2:
            x, y = q2.popleft(); comp.append(y * W + x)
            for nx, ny in ((x+1,y),(x-1,y),(x,y+1),(x,y-1)):
                kk = ny * W + nx
                if 0 <= nx < W and 0 <= ny < H and not seen[kk] and not mask[kk] and near(px[nx, ny], 10):
                    seen[kk] = 1; q2.append((nx, ny))
        if len(comp) >= 12:
            for kk in comp: mask[kk] = 1
# Frange : 3 passes, un pixel clair et peu sature voisin du fond devient du fond
for _ in range(3):
    add = []
    for y in range(1, H - 1):
        for x in range(1, W - 1):
            if mask[y * W + x]: continue
            if not (mask[y*W+x+1] or mask[y*W+x-1] or mask[(y+1)*W+x] or mask[(y-1)*W+x]): continue
            r, g, b = px[x, y]
            if min(r, g, b) > 150 and max(r, g, b) - min(r, g, b) < 40: add.append(y * W + x)
    for k in add: mask[k] = 1
rgba = Image.new("RGBA", (W, H)); rp = rgba.load()
for y in range(H):
    for x in range(W):
        rp[x, y] = (0, 0, 0, 0) if mask[y * W + x] else px[x, y] + (255,)
keyed = Image.new("RGB", (W, H), (255, 0, 255)); keyed.paste(rgba, (0, 0), rgba); keyed.save(out + "/portraits_pnj_magenta.png")
sheet = Image.new("RGBA", (3 * 56, 3 * 56), (0, 0, 0, 0))
for i in range(9):
    cx, cy = i % 3, i // 3
    cell = rgba.crop((cx * W // 3, cy * H // 3, (cx + 1) * W // 3, (cy + 1) * H // 3))
    bb = cell.getchannel("A").point(lambda v: 255 if v > 0 else 0).getbbox(); cell = cell.crop(bb)
    s = 48 / max(cell.size); w, h = max(1, round(cell.width * s)), max(1, round(cell.height * s))
    # reduction premultipliee puis alpha binaire (cle de couleur en jeu)
    p = cell.load(); pm = Image.new("RGBA", cell.size); pp = pm.load()
    for y in range(cell.height):
        for x in range(cell.width):
            r, g, b, a = p[x, y]; pp[x, y] = (r * a // 255, g * a // 255, b * a // 255, a)
    pm = pm.resize((w, h), Image.LANCZOS); sp = pm.load(); fin = Image.new("RGBA", (w, h)); fp = fin.load()
    for y in range(h):
        for x in range(w):
            r, g, b, a = sp[x, y]
            fp[x, y] = (0, 0, 0, 0) if a < 160 else (min(255, r * 255 // a), min(255, g * 255 // a), min(255, b * 255 // a), 255)
    sheet.paste(fin, (cx * 56 + 4 + (48 - w) // 2, cy * 56 + 4 + (48 - h)), fin)
sheet.save(out + "/portraits_48.png")
big = sheet.resize((sheet.width * 5, sheet.height * 5), Image.NEAREST)
bgc = Image.new("RGB", big.size, (34, 28, 22)); bgc.paste(big, (0, 0), big); bgc.save(out + "/portraits_48_apercu.png")
