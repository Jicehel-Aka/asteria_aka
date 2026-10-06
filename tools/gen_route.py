#!/usr/bin/env python3
# Route Royale (MAP_0006, loc 20, 20x18) : route pavée verticale, porte de
# Grand-Castel (remparts+tours) au nord, arrivée depuis la Mine au sud,
# campement abandonné (carnet de la caravane) sur un embranchement ouest,
# un garde (Capitaine Roland, npc 10) près de la porte.
# Tuiles : 0 herbe,1 chemin,6 arbre,19 paves,20 terre,21 rempart,22 tour,25 tonneau
import re, os
W,H=20,18
HERBE,CHEM,ARBRE,PAVES,TERRE,REMP,TOUR,TONN=0,1,6,19,20,21,22,25
g=[[HERBE for _ in range(W)] for _ in range(H)]

# bordures d'arbres gauche/droite
for y in range(H):
    g[y][0]=ARBRE; g[y][W-1]=ARBRE

# route pavée verticale (x=9,10) sur toute la hauteur
for y in range(H):
    g[y][9]=PAVES; g[y][10]=PAVES

# --- nord : porte de Grand-Castel (rempart + tours), ouverture sur la route ---
for x in range(W):
    g[0][x]=REMP
g[0][0]=TOUR; g[0][W-1]=TOUR
g[0][9]=PAVES; g[0][10]=PAVES          # passage

# --- sud : arrivée depuis la Mine (ouverture sur la route), arbres ailleurs ---
for x in range(W):
    if x not in (9,10): g[H-1][x]=ARBRE

# --- embranchement ouest vers le campement (chemin de terre) ---
for x in range(3,9):
    g[8][x]=CHEM
# campement : petite zone de terre + tonneaux
for yy in range(7,10):
    for xx in range(3,6):
        g[yy][xx]=TERRE
g[7][3]=TONN; g[9][5]=TONN

# --- embranchement est/retour forêt : chemin vers le bord gauche en bas ---
for x in range(1,9):
    g[15][x]=CHEM

# quelques arbres de décor sur l'herbe
for (x,y) in [(5,3),(14,4),(15,11),(6,12),(13,14),(4,13),(16,7)]:
    if g[y][x]==HERBE: g[y][x]=ARBRE

# départ (arrivée depuis la mine) : sur la route, en bas
PX,PY=9,16

flat=[g[y][x] for y in range(H) for x in range(W)]
arr=", ".join(str(v) for v in flat)

T=f"static const uint8_t MAP5_T[]={{{arr}}};"
# spawns : Capitaine Roland (npc 10) près de la porte
S="static const Spawn MAP5_S[]={{10,12,2}};"
# portails : porte -> Grand-Castel (loc10, gated caravane_ok) ; bas-gauche -> Forêt (loc8)
P="static const Portal MAP5_P[]={{9,0,10,15,22}, {1,15,8,10,2}};"

here=os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
hp=os.path.join(here,"components","asteria","generated","asteria_maps.h")
s=open(hp).read()

# idempotence : retire une précédente définition MAP5 si présente
s=re.sub(r"static const uint8_t MAP5_T\[\]=\{[^}]*\};\n?","",s)
s=re.sub(r"static const Spawn MAP5_S\[\]=\{.*?\};\n?","",s)
s=re.sub(r"static const Portal MAP5_P\[\]=\{.*?\};\n?","",s)

anchor="static constexpr uint8_t MAP_COUNT="
assert anchor in s
s=s.replace(anchor, T+"\n"+S+"\n"+P+"\n"+anchor)
# MAP_COUNT 5 -> 6
s=re.sub(r"static constexpr uint8_t MAP_COUNT=\d+;","static constexpr uint8_t MAP_COUNT=6;",s)
# ajoute la ligne de table si absente
if "MAP_0006" not in s:
    row=f'  {{"MAP_0006",20,{W},{H},{PX},{PY},MAP5_T,MAP5_S,1,MAP5_P,2}}'
    s=s.replace('{"MAP_0005",10,30,24,15,22,MAP4_T,MAP4_S,5,MAP4_P,2}\n};',
                '{"MAP_0005",10,30,24,15,22,MAP4_T,MAP4_S,5,MAP4_P,2},\n'+row+'\n};')
open(hp,"w").write(s)
print(f"Route Royale: {W}x{H}, depart ({PX},{PY}), MAP_COUNT=6")
PY
