#!/usr/bin/env python3
# Bake de textures pierre (depuis la photo de reference) en tuiles 64x64 RGB565 (BGR565 facon gb::rgb).
from PIL import Image, ImageEnhance
SRC='/root/.claude/uploads/e6dac36d-15b0-5431-8ea2-7998a81a6541/54c76453-image.png'
TW=TH=64
def pack(r,g,b): return ((b>>3)<<11)|((g>>2)<<5)|(r>>3)
im=Image.open(SRC).convert('RGB')
W,H=im.size
# crops varies -> tuiles distinctes (les liserés sombres entre cellules cachent les raccords)
def tile(box, rot=0, flip=False, bright=1.0, warm=1.0):
    c=im.crop(box).resize((TW,TH),Image.LANCZOS)
    if rot: c=c.rotate(rot)
    if flip: c=c.transpose(Image.FLIP_LEFT_RIGHT)
    if bright!=1.0: c=ImageEnhance.Brightness(c).enhance(bright)
    px=c.load(); out=[]
    for y in range(TH):
        for x in range(TW):
            r,g,b=px[x,y]
            if warm!=1.0:
                r=min(255,int(r*warm)); b=int(b/warm)
            out.append(pack(r,g,b))
    return out
WALLS=[
  tile((10,10,170,170)),
  tile((150,20,310,180), flip=True),
  tile((30,70,190,230), bright=0.92),
  tile((120,60,300,240), flip=True, bright=1.05),
]
FLOORS=[
  tile((40,120,200,240), bright=0.8, warm=1.12),
  tile((150,110,310,230), flip=True, bright=0.74, warm=1.12),
]
def emit(name, arrs):
    s=f"static const uint16_t {name}[{len(arrs)}][{TW*TH}]={{\n"
    for a in arrs:
        s+="{"+",".join(str(v) for v in a)+"},\n"
    s+="};\n"; return s
with open('components/asteria/generated/asteria_dungeon_tex.h','w') as f:
    f.write("#pragma once\n#include <cstdint>\nnamespace asteria { namespace dtex {\n")
    f.write(f"static const int TW={TW}, TH={TH};\n")
    f.write(f"static const int NWALL={len(WALLS)}, NFLOOR={len(FLOORS)};\n")
    f.write(emit("WALL",WALLS))
    f.write(emit("FLOOR",FLOORS))
    f.write("}}\n")
print("bake ok : %d murs, %d sols, %dx%d"%(len(WALLS),len(FLOORS),TW,TH))
