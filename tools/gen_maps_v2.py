#!/usr/bin/env python3
# ASTERIA - Cartes de liaison en ROUTAGE ORTHOGONAL (pas de diagonale "pointillee").
# Routes/sentiers = tronçons H/V relies par des coudes a angle droit -> auto-raccord propre.
# Reecrit components/asteria/generated/asteria_maps.h EN PLACE (MAP1 foret, MAP5 route, +MAP8 vallee).
import re, random
HDR = "components/asteria/generated/asteria_maps.h"
HERB=0; PATH=1; TREE=6; BRIDGE=5; WATER=16; FLOWER=17; TALL=18; ROAD=19; PAVE=20; ROCK=28

def blank(w,h,fill): return [[fill]*w for _ in range(h)]
def seg(g,tile,x0,y0,x1,y1,wd):
    # tronçon strictement H ou V, epaisseur wd (centree a droite/bas)
    if x0==x1:
        a,b=sorted((y0,y1))
        for y in range(a,b+1):
            for d in range(wd):
                x=x0+d
                if 0<=x<len(g[0]) and 0<=y<len(g): g[y][x]=tile
    else:
        a,b=sorted((x0,x1))
        for x in range(a,b+1):
            for d in range(wd):
                y=y0+d
                if 0<=x<len(g[0]) and 0<=y<len(g): g[y][x]=tile
def path_cells(segs,wd):
    cells=set()
    for (x0,y0,x1,y1) in segs:
        if x0==x1:
            a,b=sorted((y0,y1))
            for y in range(a,b+1):
                for d in range(wd): cells.add((x0+d,y))
        else:
            a,b=sorted((x0,x1))
            for x in range(a,b+1):
                for d in range(wd): cells.add((x,y0+d))
    return cells
def emit_grid(g):
    w=len(g[0]); h=len(g); flat=[]
    for r in g: flat+=r
    return w,h,",".join(str(v) for v in flat)

# ---------------- FORET DE BRUME (loc8) 32x28 : sentier orthogonal sinueux ----------------
def build_forest():
    w,h=32,28; g=blank(w,h,TREE)
    segs=[(16,0,16,8),(16,8,24,8),(24,8,24,16),(24,16,13,16),(13,16,13,27)]
    # clairieres d'herbe autour du sentier, puis le sentier
    cells=path_cells(segs,2)
    for (x,y) in list(cells):
        for dx in range(-1,2):
            for dy in range(-1,2):
                nx,ny=x+dx,y+dy
                if 0<=nx<w and 0<=ny<h and g[ny][nx]==TREE: g[ny][nx]=HERB
    for s in segs: seg(g,PATH,*s,2)
    # riviere + pont (le sentier traverse a x=24)
    ry=14
    for x in range(w): g[ry][x]=WATER
    g[ry][24]=BRIDGE; g[ry][25]=BRIDGE
    # deco
    for (x,y) in [(6,6),(28,10),(8,22),(27,22),(4,12),(20,24)]:
        if g[y][x]==TREE: g[y][x]=TALL
    # herbes cueillette SUR le sentier (le joueur les ramasse en marchant)
    herb=[(16,4),(24,11),(20,16),(13,23)]
    for x,y in herb: g[y][x]=PATH
    spawns=[(5,15,24)]                      # Lysa a cote du sentier pres de l'entree
    portals=[(13,27,0,16,2),(16,0,9,10,13)] # bas->Valbois ; haut->loc9
    start=(13,26)
    return w,h,start,g,spawns,portals,{"herb":herb,"top":(16,0),"entree":start,"lysa":(15,24)}

# ---------------- ROUTE ROYALE (loc20) 24x52 : route pavee orthogonale large ----------------
def build_route():
    w,h=24,52; g=blank(w,h,HERB)
    main=[(11,0,11,16),(11,16,18,16),(18,16,18,28),(18,28,8,28),(8,28,8,50)]
    rstub=[(11,12,23,12)]   # branche DROITE -> Vallee (portail 23,12)
    lstub=[(8,30,0,30)]     # branche GAUCHE -> Foret (portail 0,30)
    for s in main+rstub+lstub: seg(g,ROAD,*s,3)
    # arbres/rochers epars loin de la route
    road=path_cells(main+rstub+lstub,3)
    random.seed(7)
    for y in range(h):
        for x in range(w):
            if g[y][x]==HERB and min((abs(x-rx)+abs(y-ry)) for rx,ry in [(11,8),(18,22),(8,40)])>4:
                r=random.random()
                if r<0.09: g[y][x]=TREE
                elif r<0.12: g[y][x]=ROCK
                elif r<0.15: g[y][x]=TALL
    # riviere + pont (route traverse a x=8)
    ry=40
    for x in range(w): g[ry][x]=WATER
    for x in (7,8,9): g[ry][x]=BRIDGE
    # campement (paves) autour du coffre "carnet" (Q10) en (4,16)
    for yy in range(14,19):
        for xx in range(2,6):
            if g[yy][xx] not in (ROAD,): g[yy][xx]=PAVE
    g[16][4]=PAVE
    spawns=[(16,13,16),(10,17,28)]          # Tomas Venn (Q12) + brigand d'ambiance, a cote de la route
    portals=[(11,0,10,15,22),(0,30,8,10,2),(23,12,23,2,11)]  # haut->GC ; gauche->Foret ; droite->Vallee
    start=(11,1)
    return w,h,start,g,spawns,portals,{"top":(11,0),"bas":(8,50),"gauche":(0,30),"droite":(23,12)}

# ---------------- VALLEE DES PIERRES (loc23) 26x22 : chemin orthogonal + monolithes ----------------
def build_vallee():
    w,h=26,22; g=blank(w,h,HERB)
    random.seed(23)
    for y in range(h):
        for x in range(w):
            d=min(x,w-1-x,y,h-1-y)
            if d==0 or random.random()<(0.16 if d<3 else 0.05): g[y][x]=ROCK
    segs=[(0,11,8,11),(8,11,8,6),(8,6,18,6),(18,6,18,15),(18,15,23,15)]
    for s in segs: seg(g,PATH,*s,2)
    # petits embranchements vers les cases de contact des monolithes
    seg(g,PATH,5,10,5,11,1); seg(g,PATH,12,5,12,6,1); seg(g,PATH,20,8,20,15,1)
    # monolithes (rochers SOLIDES) estampes APRES le chemin, chacun avec une case de contact walkable adjacente
    mono=[(5,9),(12,4),(20,7),(21,16)]   # contacts : (5,10)U (12,5)U (20,8)U (21,15)D
    for (mx,my) in mono: g[my][mx]=ROCK
    for (x,y) in [(3,15),(23,9),(14,17)]:
        if g[y][x]==HERB: g[y][x]=FLOWER
    spawns=[(14,16,15)]                 # Selene, a cote du chemin est
    portals=[(0,11,20,22,12)]           # ouest -> retour Route
    start=(2,11)
    return w,h,start,g,spawns,portals,{"mono":mono,"entree":start}

def arr(name,csv): return f"static const uint8_t {name}[]={{{csv}}};"
def spawns_c(name,sp): return f"static const Spawn {name}[]={{"+(", ".join("{%d,%d,%d}"%(n,x,y) for n,x,y in sp) if sp else "{-1,0,0}")+"};"
def portals_c(name,po): return f"static const Portal {name}[]={{"+", ".join("{%d,%d,%d,%d,%d}"%p for p in po)+"};"

def main():
    txt=open(HDR).read()
    fw,fh,fs,fg,fsp,fpo,fi=build_forest()
    rw,rh,rs,rg,rsp,rpo,ri=build_route()
    vw,vh,vs,vg,vsp,vpo,vi=build_vallee()
    _,_,fc=emit_grid(fg); _,_,rc=emit_grid(rg); _,_,vc=emit_grid(vg)
    txt=re.sub(r'static const uint8_t MAP1_T\[\]=\{.*?\};', arr("MAP1_T",fc), txt, flags=re.S)
    txt=re.sub(r'static const uint8_t MAP5_T\[\]=\{.*?\};', arr("MAP5_T",rc), txt, flags=re.S)
    txt=re.sub(r'static const Spawn MAP1_S\[\]=\{.*?\};', spawns_c("MAP1_S",fsp), txt, flags=re.S)
    txt=re.sub(r'static const Portal MAP1_P\[\]=\{.*?\};', portals_c("MAP1_P",fpo), txt, flags=re.S)
    txt=re.sub(r'static const Spawn MAP5_S\[\]=\{.*?\};', spawns_c("MAP5_S",rsp), txt, flags=re.S)
    txt=re.sub(r'static const Portal MAP5_P\[\]=\{.*?\};', portals_c("MAP5_P",rpo), txt, flags=re.S)
    block="\n".join([arr("MAP8_T",vc), spawns_c("MAP8_S",vsp), portals_c("MAP8_P",vpo)])+"\n"
    if "MAP8_T[]" not in txt:
        txt=txt.replace("static const Map MAPS[MAP_COUNT]={", block+"static const Map MAPS[MAP_COUNT]={")
    else:
        txt=re.sub(r'static const uint8_t MAP8_T\[\]=\{.*?\};', arr("MAP8_T",vc), txt, flags=re.S)
        txt=re.sub(r'static const Spawn MAP8_S\[\]=\{.*?\};', spawns_c("MAP8_S",vsp), txt, flags=re.S)
        txt=re.sub(r'static const Portal MAP8_P\[\]=\{.*?\};', portals_c("MAP8_P",vpo), txt, flags=re.S)
    txt=re.sub(r'MAP_COUNT=8','MAP_COUNT=9',txt)
    txt=re.sub(r'\{"MAP_0002",8,[^}]*\}', '{"MAP_0002",8,%d,%d,%d,%d,MAP1_T,MAP1_S,%d,MAP1_P,%d}'%(fw,fh,fs[0],fs[1],len(fsp),len(fpo)), txt)
    txt=re.sub(r'\{"MAP_0006",20,[^}]*\}', '{"MAP_0006",20,%d,%d,%d,%d,MAP5_T,MAP5_S,%d,MAP5_P,%d}'%(rw,rh,rs[0],rs[1],len(rsp),len(rpo)), txt)
    if '"MAP_0009"' not in txt:
        txt=re.sub(r'(\{"MAP_0008",22,[^}]*\})', r'\1,\n  {"MAP_0009",23,%d,%d,%d,%d,MAP8_T,MAP8_S,%d,MAP8_P,%d}'%(vw,vh,vs[0],vs[1],len(vsp),len(vpo)), txt)
    else:
        txt=re.sub(r'\{"MAP_0009",23,[^}]*\}', '{"MAP_0009",23,%d,%d,%d,%d,MAP8_T,MAP8_S,%d,MAP8_P,%d}'%(vw,vh,vs[0],vs[1],len(vsp),len(vpo)), txt)
    open(HDR,"w").write(txt)
    print("FORET :", fi)
    print("ROUTE :", ri)
    print("VALLEE:", vi)
main()
