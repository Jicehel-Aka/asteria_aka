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

# ---------------- FORET DE BRUME (loc8) 38x34 : explorable, clairieres a herbes/champignons ----------------
def build_forest():
    w,h=38,34; g=blank(w,h,TREE)
    # sentier principal orthogonal (largeur 2) ; lignes 0-1 laissees en marge (HUD) -> sortie haut en ligne 2
    main=[(18,33,18,26),(18,26,28,26),(28,26,28,18),(28,18,10,18),(10,18,10,10),(10,10,20,10),(20,10,20,2)]
    # ramifications (largeur 1) qui s'enfoncent entre les arbres vers des clairieres de cueillette
    spurs=[(28,24,35,24),(10,14,3,14),(20,8,34,8),(20,6,12,6),(18,29,6,29),(18,31,26,31),(22,8,22,8),(18,32,20,32)]
    for s in main: seg(g,PATH,*s,2)
    for s in spurs: seg(g,PATH,*s,1)
    # riviere + pont (le sentier principal traverse a la colonne 28)
    ry=22
    for x in range(w): g[ry][x]=WATER
    g[ry][28]=BRIDGE; g[ry][29]=BRIDGE
    # noeuds de cueillette dans les clairieres (hors du sentier principal) : 9 herbes + 4 champignons
    herb=[(31,24),(34,24),(7,14),(25,8),(30,8),(16,6),(12,29),(8,29),(23,31)]
    mush=[(4,14),(33,8),(13,6),(26,31)]
    for x,y in herb: g[y][x]=FLOWER        # fleurs = herbes medicinales
    for x,y in mush: g[y][x]=TALL          # herbe haute = champignons
    spawns=[(5,20,32)]                      # Lysa pres de l'entree
    portals=[(18,33,0,16,2),(20,2,9,10,13)] # bas->Valbois ; haut->loc9 (sortie en ligne 2)
    start=(18,32)
    return w,h,start,g,spawns,portals,{"herb":herb,"mush":mush,"top":(20,2),"entree":start,"lysa":(20,32),"chest":(22,7)}

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


# ---------------- GRANDE PLAINE (loc22) 24x18 : Arc 0, la Breche au SUD, sortie NORD -> Valbois (DELTA-01) ----------------
def build_plaine():
    w,h=24,18; g=blank(w,h,HERB)
    for y in range(h):
        for x in range(w):
            if x in (0,w-1) or y in (0,1,h-1): g[y][x]=TREE      # lisiere (lignes 0-1 sous le bandeau HUD)
    # chemin orthogonal (largeur 2) : Breche (bas) -> coude -> gue sur le ruisseau -> sortie haut (ligne 2)
    segs=[(12,15,12,11),(12,11,8,11),(8,11,8,6),(8,6,12,6),(12,6,12,2)]
    for s in segs: seg(g,PATH,*s,2)
    ry=8                                                         # ruisseau est-ouest, pont sur le chemin
    for x in range(1,w-1): g[ry][x]=WATER
    g[ry][8]=BRIDGE; g[ry][9]=BRIDGE
    # la Breche : deux pierres levees de part et d'autre de l'arrivee
    for (x,y) in [(10,15),(15,15),(10,16),(15,16)]: g[y][x]=ROCK
    for (x,y) in [(4,4),(19,5),(3,13),(20,12),(17,3),(5,15)]: g[y][x]=TREE
    for (x,y) in [(6,3),(16,10),(4,10),(19,14),(14,4)]: g[y][x]=FLOWER
    for (x,y) in [(5,12),(18,4),(2,6),(21,9)]: g[y][x]=TALL
    spawns=[(21,10,10)]                  # Voyageur encapuchonne, au bord du chemin (coude)
    portals=[(12,2,0,16,24),(13,2,0,17,24)]   # sortie nord -> Valbois (arrivee au sud du village)
    start=(12,15)
    return w,h,start,g,spawns,portals,{"breche":start,"sortie":(12,2)}

# ---------------- FORGE DE DORAN (loc2) 14x12 : interieur (DELTA-01) ----------------
def build_forge():
    WALL,FLOOR,DOOR,COUNTER,TABLE,BARREL,STOOL,HEARTH,ROUND=2,5,4,23,24,25,27,30,32
    w,h=14,12; g=blank(w,h,FLOOR)
    for x in range(w): g[0][x]=WALL; g[h-1][x]=WALL
    for y in range(h): g[y][0]=WALL; g[y][w-1]=WALL
    g[h-1][7]=DOOR
    g[1][6]=HEARTH; g[1][7]=HEARTH                       # foyers de la forge
    g[4][7]=ROUND                                        # enclume
    for x in (10,11,12): g[3][x]=COUNTER                 # ratelier d'armes
    for (x,y) in [(2,4),(3,4),(2,5),(3,5)]: g[y][x]=TABLE  # etabli
    g[6][4]=STOOL
    for (x,y) in [(1,1),(12,1),(1,10),(12,10),(1,9)]: g[y][x]=BARREL   # charbon, lingots
    spawns=[(1,6,4)]                     # Doran a cote de l'enclume
    portals=[(7,11,0,12,17)]             # porte -> devant la forge, Valbois
    start=(7,10)
    return w,h,start,g,spawns,portals,{"doran":(6,4)}

# ---------------- VALBOIS (loc0) : retouches DELTA-01 sur la carte existante ----------------
def patch_valbois(txt):
    m=re.search(r'static const uint8_t MAP0_T\[\]=\{([^}]*)\};',txt); v=[int(x) for x in m.group(1).split(',')]
    W=34
    for x in range(18,28): v[23*W+x]=PATH                # allee vers la maison du sud-est (ligne 23)
    txt=txt.replace(m.group(0), arr("MAP0_T",",".join(str(t) for t in v)))
    sp=[(0,6,8),(2,17,8),(3,16,14),(6,12,17),(7,27,14),(9,22,20),(8,18,12),(20,14,20)]   # Doran (1) part dans sa forge
    po=[(16,0,8,18,32),(5,6,1,10,13),(12,16,2,7,10),(16,25,22,12,3),(17,25,22,13,3)]   # + forge, + sortie sud -> Plaine
    txt=re.sub(r'static const Spawn MAP0_S\[\]=\{.*?\};', spawns_c("MAP0_S",sp), txt, flags=re.S)
    txt=re.sub(r'static const Portal MAP0_P\[\]=\{.*?\};', portals_c("MAP0_P",po), txt, flags=re.S)
    txt=re.sub(r'\{"MAP_0001",0,([0-9]+),([0-9]+),([0-9]+),([0-9]+),MAP0_T,MAP0_S,[0-9]+,MAP0_P,[0-9]+\}',
               lambda mm:'{"MAP_0001",0,%s,%s,%s,%s,MAP0_T,MAP0_S,%d,MAP0_P,%d}'%(mm.group(1),mm.group(2),mm.group(3),mm.group(4),len(sp),len(po)), txt)
    return txt

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
    # --- DELTA-01 : Grande Plaine re-dessinee, Forge de Doran (MAP9), Valbois relie ---
    pw,ph,ps,pg,psp,ppo,pi=build_plaine(); _,_,pc=emit_grid(pg)
    txt=re.sub(r'static const uint8_t MAP7_T\[\]=\{.*?\};', arr("MAP7_T",pc), txt, flags=re.S)
    txt=re.sub(r'static const Spawn MAP7_S\[\]=\{.*?\};', spawns_c("MAP7_S",psp), txt, flags=re.S)
    txt=re.sub(r'static const Portal MAP7_P\[\]=\{.*?\};', portals_c("MAP7_P",ppo), txt, flags=re.S)
    txt=re.sub(r'\{"MAP_0008",22,[^}]*\}', '{"MAP_0008",22,%d,%d,%d,%d,MAP7_T,MAP7_S,%d,MAP7_P,%d}'%(pw,ph,ps[0],ps[1],len(psp),len(ppo)), txt)
    gw,gh,gs,gg,gsp,gpo,gi=build_forge(); _,_,gc=emit_grid(gg)
    block="\n".join([arr("MAP9_T",gc), spawns_c("MAP9_S",gsp), portals_c("MAP9_P",gpo)])+"\n"
    if "MAP9_T[]" not in txt:
        txt=txt.replace("static const Map MAPS[MAP_COUNT]={", block+"static const Map MAPS[MAP_COUNT]={")
    else:
        txt=re.sub(r'static const uint8_t MAP9_T\[\]=\{.*?\};', arr("MAP9_T",gc), txt, flags=re.S)
        txt=re.sub(r'static const Spawn MAP9_S\[\]=\{.*?\};', spawns_c("MAP9_S",gsp), txt, flags=re.S)
        txt=re.sub(r'static const Portal MAP9_P\[\]=\{.*?\};', portals_c("MAP9_P",gpo), txt, flags=re.S)
    txt=re.sub(r'MAP_COUNT=9\b','MAP_COUNT=10',txt)
    entry='{"MAP_0010",2,%d,%d,%d,%d,MAP9_T,MAP9_S,%d,MAP9_P,%d}'%(gw,gh,gs[0],gs[1],len(gsp),len(gpo))
    if '"MAP_0010"' not in txt:
        txt=re.sub(r'(\{"MAP_0009",23,[^}]*\})', lambda mm: mm.group(1)+',\n  '+entry, txt)
    else:
        txt=re.sub(r'\{"MAP_0010",2,[^}]*\}', entry, txt)
    txt=patch_valbois(txt)
    open(HDR,"w").write(txt)
    print("PLAINE:", pi)
    print("FORGE :", gi)
    print("FORET :", fi)
    print("ROUTE :", ri)
    print("VALLEE:", vi)
main()
