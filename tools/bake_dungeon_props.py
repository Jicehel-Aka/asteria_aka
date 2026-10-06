#!/usr/bin/env python3
# Sprites de decor du donjon (billboards) bakes en RGB565 (BGR565 facon gb::rgb).
# Chaque prop : grille initialisee a KEY (transparent), puis dessin pixel-art.
KEY = ((255>>3)<<11)|((0>>2)<<5)|(255>>3)   # magenta 0xF81F
def pack(r,g,b): return ((int(b)>>3)<<11)|((int(g)>>2)<<5)|(int(r)>>3)

def grid(w,h): return [[KEY]*w for _ in range(h)]
def px(g,x,y,c):
    if 0<=y<len(g) and 0<=x<len(g[0]): g[y][x]=c
def rect(g,x0,y0,x1,y1,c):
    for y in range(y0,y1+1):
        for x in range(x0,x1+1): px(g,x,y,c)
def vrect(g,x0,y0,x1,y1,ctop,cbot):
    h=max(1,y1-y0)
    for y in range(y0,y1+1):
        t=(y-y0)/h
        c=pack(ctop[0]+(cbot[0]-ctop[0])*t, ctop[1]+(cbot[1]-ctop[1])*t, ctop[2]+(cbot[2]-ctop[2])*t)
        for x in range(x0,x1+1): px(g,x,y,c)

WD=(60,40,22); WM=(110,74,40); WL=(150,104,60); WH=(182,136,82)
MT=(92,92,104); MH=(150,150,162)
RD=(66,62,56); RM=(108,102,94); RL=(150,144,134)
BONE=(212,206,186); BONES=(150,145,125)

def wood_post(g,x0,x1,y0,y1):
    vrect(g,x0,y0,x1,y1,WL,WM)
    for y in range(y0,y1+1):
        px(g,x0,y,pack(*WD)); px(g,x1,y,pack(*WD))
    for y in range(y0,y1+1,5): px(g,x0+1,y,pack(*WH))   # veinage leger

# ---- ETAI : cadre de soutenement (2 montants + linteau + jambes de force) ----
def timber():
    w,h=58,54; g=grid(w,h); t=6
    wood_post(g,2,2+t,6,h-1)             # montant gauche
    wood_post(g,w-3-t,w-3,6,h-1)         # montant droit
    vrect(g,2,0,w-3,t+2, WH, WM)         # linteau
    for x in range(2,w-2): px(g,x,0,pack(*WD)); px(g,x,t+2,pack(*WD))
    # jambes de force (diagonales en haut)
    for i in range(10):
        px(g,2+t+1+i, t+3+i, pack(*WM)); px(g,2+t+2+i, t+3+i, pack(*WD))
        px(g,w-4-t-1-i, t+3+i, pack(*WM)); px(g,w-5-t-1-i, t+3+i, pack(*WD))
    # boulons
    for (bx,by) in [(2+t//2,10),(w-3-t//2,10),(6,t//1),(w-7,t//1)]:
        px(g,bx,by,pack(*MH)); px(g,bx+1,by,pack(*MT))
    return w,h,g,95

# ---- TONNEAU ----
def barrel():
    w,h=20,26; g=grid(w,h)
    # corps bombe
    for y in range(h):
        t=y/(h-1); bulge=int(2*(1-abs(t-0.5)*2))     # plus large au milieu
        x0=2-bulge; x1=w-3+bulge
        tt=0.5+0.5*((x1-x0)-(w-5))/4
        vrect(g,max(0,x0),y,min(w-1,x1),y,WL,WM)
    for x in range(w):  # douves
        if x%3==0:
            for y in range(1,h-1):
                if g[y][x]!=KEY: px(g,x,y,pack(*WD))
    rect(g,0,3,w-1,4,pack(*MT)); rect(g,0,h-5,w-1,h-4,pack(*MT))   # cercles metal
    rect(g,0,3,w-1,3,pack(*MH)); rect(g,0,h-5,w-1,h-5,pack(*MH))
    return w,h,g,55

# ---- CAISSE ----
def crate():
    w,h=24,22; g=grid(w,h)
    vrect(g,0,0,w-1,h-1,WL,WM)
    for x in (0,w-1):
        for y in range(h): px(g,x,y,pack(*WD))
    for y in (0,h-1):
        for x in range(w): px(g,x,y,pack(*WD))
    # planches + croix
    rect(g,0,h//3,w-1,h//3,pack(*WD)); rect(g,0,2*h//3,w-1,2*h//3,pack(*WD))
    for i in range(min(w,h)):
        px(g,1+i,1+i*(h-2)//(w-2),pack(*WH)); px(g,w-2-i,1+i*(h-2)//(w-2),pack(*WD))
    return w,h,g,52

# ---- GRAVATS / EBOULIS ----
def rubble():
    import random; random.seed(3)
    w,h=34,18; g=grid(w,h)
    rocks=[(5,13,5),(11,12,6),(18,13,5),(24,12,6),(29,14,4),(9,15,4),(21,15,4),(15,14,5),(26,15,4)]
    for (cx,cy,r) in rocks:
        for y in range(cy-r,cy+r+1):
            for x in range(cx-r,cx+r+1):
                if (x-cx)**2+(y-cy)**2<=r*r:
                    t=(y-(cy-r))/(2*r)
                    base=RM if (x+y)%5 else RD
                    c=pack(base[0]+(RD[0]-RL[0])*(t-0.5)*0.6, base[1], base[2])
                    px(g,x,y,c)
        for x in range(cx-r,cx+r): px(g,x,cy-r+1,pack(*RL))   # reflet haut
    return w,h,g,34

# ---- CRANE ----
def skull():
    w,h=14,14; g=grid(w,h)
    # calotte
    for y in range(0,9):
        for x in range(0,w):
            if (x-7)**2/40+(y-6)**2/36<=1: px(g,x,y,pack(*BONE))
    # machoire
    rect(g,4,9,9,12,pack(*BONE))
    # yeux + nez
    rect(g,3,5,5,7,pack(10,8,8)); rect(g,8,5,10,7,pack(10,8,8))
    px(g,6,8,pack(20,16,16)); px(g,7,8,pack(20,16,16))
    for x in range(4,10,2): px(g,x,12,pack(*BONES))   # dents
    return w,h,g,24

PROPS=[("TIMBER",timber()),("BARREL",barrel()),("CRATE",crate()),("RUBBLE",rubble()),("SKULL",skull())]

def emit(name,w,h,g):
    flat=",".join(str(g[y][x]) for y in range(h) for x in range(w))
    return f"static const uint16_t {name}[{w*h}]={{{flat}}};\n"

with open('components/asteria/generated/asteria_dungeon_props.h','w') as f:
    f.write("#pragma once\n#include <cstdint>\nnamespace asteria { namespace dprop {\n")
    f.write(f"static const uint16_t KEY={KEY};\n")
    f.write(f"static const int NPROP={len(PROPS)};\n")
    names=[]
    PW=[];PH=[];PF=[]
    for nm,(w,h,g,hf) in PROPS:
        f.write(emit(nm,w,h,g)); names.append(nm); PW.append(w);PH.append(h);PF.append(hf)
    f.write("static const uint16_t* const PROP[]={"+",".join(names)+"};\n")
    f.write("static const int PW[]={"+",".join(map(str,PW))+"};\n")
    f.write("static const int PH[]={"+",".join(map(str,PH))+"};\n")
    f.write("static const int PF[]={"+",".join(map(str,PF))+"}; // hauteur en % de la case\n")
    # index symboliques
    f.write("enum { P_TIMBER=0, P_BARREL=1, P_CRATE=2, P_RUBBLE=3, P_SKULL=4 };\n")
    f.write("}}\n")
print("props bakes :", ", ".join(n for n,_ in PROPS))
