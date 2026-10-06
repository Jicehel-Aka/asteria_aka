#!/usr/bin/env python3
# Taverne MAP_0004 (20x15) bien remplie : bar + bouteilles, 4 grandes tables 2x2
# avec tabourets et clients attables, tonneaux dans les coins. Sortie devant la porte.
import re
W,H=20,15
WALL,FLOOR,DOOR=2,5,4
COUNTER,TABLE,BARREL,BOTTLE,STOOL=23,24,25,26,27
g=[[FLOOR]*W for _ in range(H)]
def put(x,y,t):
    if 0<=x<W and 0<=y<H: g[y][x]=t
def free(x,y): return 0<=x<W and 0<=y<H and g[y][x]==FLOOR
for x in range(W): put(x,0,WALL); put(x,H-1,WALL)
for y in range(H): put(0,y,WALL); put(W-1,y,WALL)
put(10,H-1,DOOR)

# --- bar : etagere a bouteilles (fond) + comptoir avec une ouverture en x=10 ---
for x in range(3,17): put(x,1,BOTTLE)
for x in range(3,17):
    if x!=10: put(x,3,COUNTER)          # ouverture au milieu pour parler a l'aubergiste
# tabourets de bar (cote client)
for x in (4,6,8,12,14):
    put(x,4,STOOL)

# --- tonneaux dans les coins ---
for (x,y) in [(17,1),(18,1),(18,2),(1,12),(2,12),(1,11),(18,12),(17,12)]:
    put(x,y,BARREL)

spawns=[(13,10,2)]   # aubergiste derriere l'ouverture du bar (parler depuis (10,3))
patron_pool=[4,6,8,11,12,15,16,0]; pi=0
# --- 4 grandes tables 2x2 + tabourets + clients ---
tables=[(3,7),(15,7),(3,10),(15,10)]
for (x0,y0) in tables:
    for dx in (0,1):
        for dy in (0,1): put(x0+dx,y0+dy,TABLE)
    stools=[(x0-1,y0),(x0+2,y0+1),(x0+1,y0-1),(x0,y0+2)]
    for (sx,sy) in stools:
        if free(sx,sy): put(sx,sy,STOOL)
    # 2 clients par table (sur les tabourets gauche/droite)
    for (sx,sy) in (stools[0],stools[1]):
        if g[sy][sx]==STOOL and pi<len(patron_pool):
            spawns.append((patron_pool[pi],sx,sy)); pi+=1

# joueur de des attable (table bas-gauche), a une place libre
spawns.append((10, 3-0+0, 0))   # placeholder remplace juste apres
spawns[-1]=(10, 4, 10)          # npc10 (des) sur le tabouret de bar (4,10)? -> non: (4,10) libre?
# place le des sur un tabouret de bar libre
dice_placed=False
for (sx,sy) in [(8,4),(12,4),(6,4),(14,4)]:
    if g[sy][sx]==STOOL:
        spawns[-1]=(10,sx,sy); dice_placed=True; break
if not dice_placed: spawns[-1]=(10,8,4)

portals=[(10,14,0,5,7)]   # sortie devant la porte de l'auberge Valbois (5,6)->(5,7)
start=(10,13)

flat=','.join(str(g[y][x]) for y in range(H) for x in range(W))
sp=', '.join('{%d,%d,%d}'%s for s in spawns)
po=', '.join('{%d,%d,%d,%d,%d}'%p for p in portals)
h=open('components/asteria/generated/asteria_maps.h').read()
h=re.sub(r'MAP3_T\[\]=\{[^;]*\};','MAP3_T[]={'+flat+'};',h,count=1)
h=re.sub(r'MAP3_S\[\]=\{[^;]*\};','MAP3_S[]={'+sp+'};',h,count=1)
h=re.sub(r'MAP3_P\[\]=\{[^;]*\};','MAP3_P[]={'+po+'};',h,count=1)
h=re.sub(r'\{"MAP_0004",1,20,15,\d+,\d+,MAP3_T,MAP3_S,\d+,MAP3_P,\d+\}',
         '{"MAP_0004",1,20,15,%d,%d,MAP3_T,MAP3_S,%d,MAP3_P,%d}'%(start[0],start[1],len(spawns),len(portals)),h,count=1)
open('components/asteria/generated/asteria_maps.h','w').write(h)
print("Taverne v2 : spawns=%d (aubergiste+des+%d clients)"%(len(spawns),len(spawns)-2))
