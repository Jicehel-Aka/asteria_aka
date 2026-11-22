#!/usr/bin/env python3
# Rebake la banque PNJ (down_1 de chaque PNJ) -> asteria_npc.h, RGB565 (BGR565, cle magenta),
# downscale LANCZOS premultiplie (evite les halos magenta), hauteur alignee sur le heros (20px).
# Corrige le Voyageur (sprite dedie VOYAGEUR au lieu du placeholder) et Dame Orwenn (sprite feminin).
import os, re
from PIL import Image
BANK="/mnt/attach/outputs/ASTERIA/game/assets/npc"
OUT="components/asteria/generated/asteria_npc.h"
H=20; WRAP=24
def bgr565(r,g,b): return ((b>>3)<<11)|((g>>2)<<5)|(r>>3)
KEY=bgr565(255,0,255)
def bake(path):
    im=Image.open(path).convert("RGBA")
    bb=im.getbbox()
    if bb: im=im.crop(bb)
    w=max(1,round(im.width*H/im.height))
    # premultiplie -> LANCZOS -> depremultiplie : pas de frange
    px=im.load()
    pm=Image.new("RGBA",im.size)
    pp=pm.load()
    for y in range(im.height):
        for x in range(im.width):
            r,g,b,a=px[x,y]; pp[x,y]=(r*a//255,g*a//255,b*a//255,a)
    pm=pm.resize((w,H),Image.LANCZOS)
    q=pm.load(); out=[]
    for y in range(H):
        for x in range(w):
            r,g,b,a=q[x,y]
            if a<128: out.append(KEY)
            else:
                r=min(255,r*255//a); g=min(255,g*255//a); b=min(255,b*255//a)
                out.append(bgr565(r,g,b))
    return w,out
def arr_c(name,w,vals):
    rows=[",".join(f"0x{v:04X}" for v in vals[i:i+WRAP]) for i in range(0,len(vals),WRAP)]
    return f"static const uint16_t {name}[] = {{\n  "+",\n  ".join(rows)+"\n};", w

# slots 0..17 = NPC_0001..NPC_0018 (ordre du registre asteria_gen.h) ; 18 = VOYAGEUR
srcs=[f"{BANK}/NPC_{i:04d}/down_1.png" for i in range(1,19)]+[f"{BANK}/VOYAGEUR/down_1.png"]
L=["#pragma once","#include <cstdint>","namespace asteria { namespace spr {",
   "#ifndef ASTERIA_SPR_DEFS","#define ASTERIA_SPR_DEFS",
   "struct Sprite { const uint16_t* px; int16_t w; int16_t h; };",
   f"static const uint16_t KEY = 0x{KEY:04X};","#endif"]
dims=[]
for i,s in enumerate(srcs):
    w,vals=bake(s); code,_=arr_c(f"NPC_{i}",w,vals); L.append(code); dims.append((f"NPC_{i}",w))
# table : slots 0..17 arts directs ; 18 Racha->NPC_16(Tomas Venn marchand) ; 19 Meluve->NPC_9(Edric) ;
# 20 Dame Orwenn->NPC_12(Soeur Helena, feminin) ; 21 Voyageur->NPC_18(art dedie)
tbl=[]
for i in range(18): tbl.append(f"  {{NPC_{i},{dims[i][1]},{H}}}")
tbl.append(f"  {{NPC_16,{dims[16][1]},{H}}}   /*18 Racha: reutilise Tomas Venn*/")
tbl.append(f"  {{NPC_9,{dims[9][1]},{H}}}    /*19 Meluve: reutilise Edric*/")
tbl.append(f"  {{NPC_12,{dims[12][1]},{H}}}   /*20 Dame Orwenn: reutilise Soeur Helena (feminin)*/")
tbl.append(f"  {{NPC_18,{dims[18][1]},{H}}}   /*21 Voyageur: art dedie de la banque*/")
L.append(f"static const int16_t NPC_COUNT=22;")
L.append("static const Sprite NPC[22] = {\n"+",\n".join(tbl)+"\n};")
L.append("} }")
open(OUT,"w").write("\n".join(L)+"\n")
print("asteria_npc.h rebaked:", len(srcs), "arts banque, hauteur", H)
print("dims:", dims)
