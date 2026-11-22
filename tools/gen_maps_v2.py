#!/usr/bin/env python3
# ASTERIA - Agrandissement des cartes de liaison (foret, route) + Vallee des Pierres dediee.
# Reecrit components/asteria/generated/asteria_maps.h EN PLACE (MAP1 foret, MAP5 route, +MAP8 vallee),
# et imprime les coords de couplage a recabler (herbes, monolithes, sorties donjon, portails).
import re, math, os
HDR = "components/asteria/generated/asteria_maps.h"
# tuiles : 0 herbe,1 chemin,2 mur,3 eau,5 pont,6 arbre,16 eau_nat,17 fleurs,18 herbe_haute,19 route,20 paves,28 rocher
HERB=0; PATH=1; TREE=6; BRIDGE=5; WATER=16; FLOWER=17; TALL=18; ROAD=19; PAVE=20; ROCK=28

def blank(w,h,fill): return [[fill]*w for _ in range(h)]
def pathx(y, cx, amp, period, w, margin=3):
    return max(margin, min(w-1-margin, int(round(cx + amp*math.sin(y*2*math.pi/period)))))

def emit_grid(g):
    w=len(g[0]); h=len(g); flat=[]
    for row in g: flat+=row
    return w,h,",".join(str(v) for v in flat)

# ---------------- FORET DE BRUME (loc8) : 32x28, sinueuse ----------------
def build_forest():
    w,h=32,28
    g=blank(w,h,TREE)
    cxs=[]
    for y in range(h):
        cx=pathx(y,16,7,22,w); cxs.append(cx)
        # clairiere d'herbe autour du chemin
        for dx in range(-3,4):
            x=cx+dx
            if 0<=x<w: g[y][x]=HERB
        g[y][cx]=PATH
        if cx+1<w: g[y][cx+1]=PATH
    # riviere + pont
    ry=14
    for x in range(w): g[ry][x]=WATER
    g[ry][cxs[ry]]=BRIDGE;
    if cxs[ry]+1<w: g[ry][cxs[ry]+1]=BRIDGE
    # quelques fleurs/herbes hautes deco
    for (x,y) in [(6,6),(25,9),(8,20),(24,22),(10,4),(22,17)]:
        if g[y][x]==TREE: g[y][x]=TALL
    # cellules d'herbe cueillette (adjacentes au chemin, a des y varies)
    herb_cells=[]
    for y in (4,9,19,24):
        x=cxs[y]+2
        if x>=w-1: x=cxs[y]-2
        g[y][x]=HERB; herb_cells.append((x,y))
    # entree bas (depuis Valbois) et portail haut (vers loc9)
    bx=cxs[h-2]; g[h-1][cxs[h-1]]=PATH
    top=cxs[0]; g[0][top]=PATH
    spawns=[(5, cxs[h-3], h-3)]   # Lysa pres de l'entree
    # portails : {x,y,target_loc,sx,sy}
    portals=[(cxs[h-1],h-1,0,16,2), (top,0,9,10,13)]
    start=(bx,h-2)
    return (w,h,start,g,spawns,portals,herb_cells,top)

# ---------------- ROUTE ROYALE (loc20) : 24x52, longue et sinueuse ----------------
def build_route():
    w,h=24,52
    g=blank(w,h,HERB)
    cxs=[]
    for y in range(h):
        cx=pathx(y,11,6,30,w); cxs.append(cx)
        g[y][cx]=ROAD
        if cx+1<w: g[y][cx+1]=ROAD
    # arbres et rochers epars sur les bas-cotes
    import random; random.seed(7)
    for y in range(h):
        for x in range(w):
            if g[y][x]==HERB and abs(x-cxs[y])>3:
                r=random.random()
                if r<0.10: g[y][x]=TREE
                elif r<0.14: g[y][x]=ROCK
                elif r<0.17: g[y][x]=TALL
    # riviere + pont (bas)
    ry=38
    for x in range(w): g[ry][x]=WATER
    g[ry][cxs[ry]]=BRIDGE
    if cxs[ry]+1<w: g[ry][cxs[ry]+1]=BRIDGE
    # campement (paves) autour du coffre "carnet de la caravane" (Q10) en (4,16)
    for yy in range(15,19):
        for xx in range(2,6):
            if 0<=xx<w and g[yy][xx]!=ROAD: g[yy][xx]=PAVE
    g[16][4]=PAVE  # la case du coffre reste walkable
    # branche GAUCHE -> Foret (loc8), vers y=30
    ly=30
    for x in range(0,cxs[ly]):
        g[ly][x]=ROAD
    # branche DROITE -> Vallee (loc23), vers y=12
    vy=12
    for x in range(cxs[vy]+1,w):
        g[vy][x]=ROAD
    # nettoyer bords
    top=cxs[0]; g[0][top]=ROAD
    bot=cxs[h-2]
    spawns=[(16,cxs[6],6),(10,cxs[24],24)]  # Tomas Venn (Q12) + brigand d'ambiance
    # portails
    portals=[(top,0,10,15,22),        # haut -> Grand-Castel
             (0,ly,8,10,2),           # gauche -> Foret
             (w-1,vy,23,2,11)]        # droite -> Vallee des Pierres
    start=(bot,h-2)
    return (w,h,start,g,spawns,portals,top,bot,ly,vy)

# ---------------- VALLEE DES PIERRES (loc23, NOUVELLE) : 26x22 ----------------
def build_vallee():
    w,h=26,22
    g=blank(w,h,HERB)
    # relief rocheux : beaucoup de rochers sur les bords
    import random; random.seed(23)
    for y in range(h):
        for x in range(w):
            d=min(x,w-1-x,y,h-1-y)
            if d==0: g[y][x]=ROCK
            elif random.random()< (0.18 if d<3 else 0.05): g[y][x]=ROCK
    # chemin central sinueux ouest->est
    cys=[]
    for x in range(w):
        cy=max(2,min(h-3,int(round(11+4*math.sin(x*2*math.pi/20))))); cys.append(cy)
        g[cy][x]=PATH; g[cy-1][x]=HERB
        if cy+1<h: g[cy][x]=PATH
    # entree ouest (depuis la route) : corridor plat row 11, puis descente vers le chemin central
    for x in range(0,6): g[11][x]=PATH
    y=11
    while y!=cys[5]:
        y += 1 if cys[5]>y else -1
        g[y][5]=PATH
    # 4 monolithes (rochers) adjacents au chemin -> Pierres Murmurantes (Q13)
    mono=[]
    for x in (5,11,17,22):
        cy=cys[x]; my=cy-1
        g[my][x]=ROCK; mono.append((x,my))   # le joueur fait face depuis cy
        g[cys[x]][x]=PATH
    # fleurs deco
    for (x,y) in [(4,15),(20,6),(13,16)]:
        if g[y][x]==HERB: g[y][x]=FLOWER
    spawns=[(14, 20, 11)]  # Selene (cartographe) guide vers les pierres
    portals=[(0,11,20,22,12)]  # ouest -> retour Route Royale (loc20) pres de la branche droite
    start=(2,11)
    return (w,h,start,g,spawns,portals,mono)

def arr(name, csv): return f"static const uint8_t {name}[]={{{csv}}};"
def spawns_c(name,sp): return f"static const Spawn {name}[]={{"+(", ".join("{%d,%d,%d}"%(n,x,y) for n,x,y in sp) if sp else "{-1,0,0}")+"};"
def portals_c(name,po): return f"static const Portal {name}[]={{"+", ".join("{%d,%d,%d,%d,%d}"%p for p in po)+"};"

def main():
    txt=open(HDR).read()
    F=build_forest(); R=build_route(); V=build_vallee()
    fw,fh,fstart,fg,fsp,fpo,herb,ftop=F
    rw,rh,rstart,rg,rsp,rpo,rtop,rbot,rly,rvy=R
    vw,vh,vstart,vg,vsp,vpo,mono=V
    _,_,fcsv=emit_grid(fg); _,_,rcsv=emit_grid(rg); _,_,vcsv=emit_grid(vg)

    # remplacer MAP1_T, MAP5_T
    txt=re.sub(r'static const uint8_t MAP1_T\[\]=\{.*?\};', arr("MAP1_T",fcsv), txt, flags=re.S)
    txt=re.sub(r'static const uint8_t MAP5_T\[\]=\{.*?\};', arr("MAP5_T",rcsv), txt, flags=re.S)
    # remplacer spawns/portals MAP1, MAP5
    txt=re.sub(r'static const Spawn MAP1_S\[\]=\{.*?\};', spawns_c("MAP1_S",fsp), txt, flags=re.S)
    txt=re.sub(r'static const Portal MAP1_P\[\]=\{.*?\};', portals_c("MAP1_P",fpo), txt, flags=re.S)
    txt=re.sub(r'static const Spawn MAP5_S\[\]=\{.*?\};', spawns_c("MAP5_S",rsp), txt, flags=re.S)
    txt=re.sub(r'static const Portal MAP5_P\[\]=\{.*?\};', portals_c("MAP5_P",rpo), txt, flags=re.S)
    # ajouter MAP8 (vallee) juste avant la table MAPS
    block = "\n".join([arr("MAP8_T",vcsv), spawns_c("MAP8_S",vsp), portals_c("MAP8_P",vpo)])+"\n"
    txt=txt.replace("static const Map MAPS[MAP_COUNT]={", block+"static const Map MAPS[MAP_COUNT]={")
    # MAP_COUNT 8->9
    txt=re.sub(r'MAP_COUNT=8','MAP_COUNT=9',txt)
    # table : MAP1 (foret) et MAP5 (route) nouvelles dims + spawn_n/portal_n ; ajouter vallee
    txt=re.sub(r'\{"MAP_0002",8,[^}]*\}', '{"MAP_0002",8,%d,%d,%d,%d,MAP1_T,MAP1_S,%d,MAP1_P,%d}'%(fw,fh,fstart[0],fstart[1],len(fsp),len(fpo)), txt)
    txt=re.sub(r'\{"MAP_0006",20,[^}]*\}', '{"MAP_0006",20,%d,%d,%d,%d,MAP5_T,MAP5_S,%d,MAP5_P,%d}'%(rw,rh,rstart[0],rstart[1],len(rsp),len(rpo)), txt)
    # inserer la ligne vallee apres MAP_0008
    txt=re.sub(r'(\{"MAP_0008",22,[^}]*\})', r'\1,\n  {"MAP_0009",23,%d,%d,%d,%d,MAP8_T,MAP8_S,%d,MAP8_P,%d}'%(vw,vh,vstart[0],vstart[1],len(vsp),len(vpo)), txt)

    open(HDR,"w").write(txt)
    print("=== asteria_maps.h reecrit ===")
    print("FORET 32x28  herbes(loc8) ->", herb, " top(->loc9):",(ftop,0), " entree(start):",fstart)
    print("ROUTE 24x52  top(->loc10):",(rtop,0)," gauche y=",rly," droite(->vallee) y=",rvy," bas(start/mine):",rstart)
    print("VALLEE 26x22 monolithes(loc23) ->", mono, " entree:",vstart)
main()
