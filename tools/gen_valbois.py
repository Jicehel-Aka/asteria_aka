#!/usr/bin/env python3
# Generateur Valbois "organique" : batiments decales, rues qui serpentent.
import re, random
from collections import deque
W,H=34,26
random.seed(7)
G=[[0]*W for _ in range(H)]          # 0 = herbe
SOLID=[0,0,1,1,0,0,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,1,1]
def inb(x,y): return 0<=x<W and 0<=y<H
def put(x,y,t):
    if inb(x,y): G[y][x]=t

# ---- bordure d'arbres irreguliere (trou d'entree en haut x16-17) ----
for x in range(W):
    for y in range(H):
        edge = x<2 or x>=W-2 or y<2 or y>=H-2
        if not edge: continue
        if y<=1 and x in (16,17): continue          # porte nord
        # densite : plein sur l'extreme bord, clairseme sur 2e rangee
        onrim = x==0 or x==W-1 or y==0 or y==H-1
        if onrim or random.random()<0.55:
            put(x,y,6)

# ---- batiments : (bx,by,w,rh,roof,door_i,sign_i) implantation decalee ----
# rh = rangees de toit ; la facade est la rangee by+rh
# mat : materiau UNIQUE du pan de facade (2 pierre, 13 brique, 14 bois, 15 crepi)
B=[
 (3 ,3 ,6,3, 9,2,3,15),   # Auberge  - crepi
 (23,2 ,6,3,11,3,-1,2),   # Mairie   - pierre
 (15,5 ,5,2,10,2,3,14),   # Boutique - bois
 (4 ,12,5,2, 7,1,-1,13),  # brique
 (26,11,5,2, 7,2,-1,15),  # crepi
 (11,14,4,2, 7,1,-1,2),   # pierre
 (20,16,5,3, 7,2,-1,13),  # brique
 (5 ,20,4,2, 7,2,-1,14),  # bois
 (28,19,4,2, 7,1,-1,15),  # crepi
]
doors=[]   # (abs_x, abs_y, kind_roof)
for (bx,by,w,rh,roof,di,si,mat) in B:
    for ry in range(by,by+rh):
        for rx in range(bx,bx+w):
            put(rx,ry,roof)
    fy=by+rh
    for i in range(w): put(bx+i,fy,mat)          # tout le pan : meme materiau
    for i in range(w):                           # fenetres par-dessus (cellules paires libres)
        if i==di or i==si: continue
        if i%2==0: put(bx+i,fy,8)
    put(bx+di,fy,4)                              # porte par-dessus
    if si>=0: put(bx+si,fy,12)                   # enseigne par-dessus
    doors.append((bx+di,fy,roof))

building_mask=[[SOLID[G[y][x]]==1 for x in range(W)] for y in range(H)]

# ---- fontaine (plaza) decentree ----
fx,fy=16,10
for dx in range(-1,3):
    for dy in range(-1,3):
        if inb(fx+dx,fy+dy) and not building_mask[fy+dy][fx+dx]:
            put(fx+dx,fy+dy,19)                 # paves autour
for dx in (0,1):
    for dy in (0,1):
        put(fx+dx,fy+dy,3)                      # bassin carre

# ---- route principale sinueuse (serpente), largeur 2 ----
WP=[(16,1),(15,5),(13,9),(15,12),(19,15),(17,19),(16,25)]
def carve2(x,y):
    for ox in (0,1):
        for oy in (0,1):
            if inb(x+ox,y+oy) and G[y+oy][x+ox] in (0,17,18,20,6):
                G[y+oy][x+ox]=1
def line(a,b):
    x,y=a; bx,by=b; turn=0
    carve2(x,y)
    while (x,y)!=(bx,by):
        dx=(bx>x)-(bx<x); dy=(by>y)-(by<y)
        # staircase : alterne les axes pour un trace diagonal
        if dx and dy:
            if turn%2==0: x+=dx
            else: y+=dy
            turn+=1
        elif dx: x+=dx
        else: y+=dy
        carve2(x,y)
for i in range(len(WP)-1):
    line(WP[i],WP[i+1])

# ---- relier chaque porte a la route par BFS (chemin le plus court) ----
def bfs_to_path(start):
    sx,sy=start
    seen={(sx,sy)}; q=deque([(sx,sy,[(sx,sy)])])
    while q:
        x,y,pa=q.popleft()
        if G[y][x]==1 and (x,y)!=start:
            return pa
        for dx,dy in((0,1),(0,-1),(1,0),(-1,0)):
            nx,ny=x+dx,y+dy
            if inb(nx,ny) and (nx,ny) not in seen and not building_mask[ny][nx] and G[ny][nx]!=3:
                seen.add((nx,ny)); q.append((nx,ny,pa+[(nx,ny)]))
    return None
for (dx,dy,roof) in doors:
    front=(dx,dy+1)
    if not inb(*front) or building_mask[front[1]][front[0]]: continue
    pa=bfs_to_path(front)
    if pa:
        for (x,y) in pa:
            if G[y][x] in (0,17,18,20,6): G[y][x]=1

# ---- mare naturelle organique (zone degagee gauche-milieu) ----
pond=[(4,16),(5,16),(6,16),(4,17),(5,17),(6,17),(7,17),(5,18),(6,18),(3,17)]
for (x,y) in pond:
    if inb(x,y) and G[y][x] in (0,17,18,20): put(x,y,16)
put(7,16,6); put(3,16,6)   # arbres au bord de la mare

# ---- quelques arbres isoles dans le bourg (moins "vide"/geometrique) ----
for (x,y) in [(20,6),(9,13),(30,16),(13,21),(24,9),(19,22)]:
    if inb(x,y) and G[y][x]==0: put(x,y,6)

# ---- deco eparse (fleurs/herbes hautes/terre) sur herbe libre ----
for _ in range(60):
    x=random.randrange(2,W-2); y=random.randrange(2,H-2)
    if G[y][x]==0:
        r=random.random()
        G[y][x]=17 if r<0.5 else (18 if r<0.8 else 0)
# un etal de marche (terre) decale
for x in range(24,28):
    if G[22][x]==0: put(x,22,20)

# ---- spawns PNJ sur tuiles marchables, pres de leurs batiments ----
def nearest_walk(x,y):
    for rad in range(0,6):
        for ddy in range(-rad,rad+1):
            for ddx in range(-rad,rad+1):
                nx,ny=x+ddx,y+ddy
                if inb(nx,ny) and SOLID[G[ny][nx]]==0 and G[ny][nx]!=3 and G[ny][nx]!=16:
                    return nx,ny
    return x,y
# npc2 = marchand (devant boutique, porte abs (17,7)), autres varies
want=[(0,(6,8)),(2,(17,8)),(3,(16,14)),(6,(12,17)),(7,(27,14)),(9,(22,20))]
spawns=[]
for npc,(x,y) in want:
    wx,wy=nearest_walk(x,y); spawns.append((npc,wx,wy))

# ---- portails ----
put(16,0,1)  # sortie nord doit etre marchable
inn_door=doors[0]  # auberge
portals=[(16,0,8,10,13),(inn_door[0],inn_door[1],1,10,13)]

# ---- start marchable ----
startx,starty=16,23
if SOLID[G[starty][startx]]==1:
    startx,starty=nearest_walk(16,23)

# ---- emission ----
flat=','.join(str(G[y][x]) for y in range(H) for x in range(W))
sp=', '.join('{%d,%d,%d}'%(n,x,y) for (n,x,y) in spawns)
po=', '.join('{%d,%d,%d,%d,%d}'%p for p in portals)
hdr=open('components/asteria/generated/asteria_maps.h').read()
hdr=re.sub(r'MAP0_T\[\]=\{[^;]*\};', 'MAP0_T[]={'+flat+'};', hdr, count=1)
hdr=re.sub(r'MAP0_S\[\]=\{[^;]*\};', 'MAP0_S[]={'+sp+'};', hdr, count=1)
hdr=re.sub(r'MAP0_P\[\]=\{[^;]*\};', 'MAP0_P[]={'+po+'};', hdr, count=1)
# maj header px,py + spawn_n + portal_n de MAP_0001
hdr=re.sub(r'\{"MAP_0001",0,34,26,\d+,\d+,MAP0_T,MAP0_S,\d+,MAP0_P,\d+\}',
           '{"MAP_0001",0,34,26,%d,%d,MAP0_T,MAP0_S,%d,MAP0_P,%d}'%(startx,starty,len(spawns),len(portals)),
           hdr, count=1)
open('components/asteria/generated/asteria_maps.h','w').write(hdr)
print("Valbois regenere : spawns=%d portals=%d start=(%d,%d)"%(len(spawns),len(portals),startx,starty))
print("portes:",doors)
