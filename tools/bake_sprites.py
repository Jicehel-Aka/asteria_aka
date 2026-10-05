#!/usr/bin/env python3
# ASTERIA — Sprite baker : PNG -> en-tete C++ RGB565 (format facade gb::, couleur-cle magenta).
# Sortie LISIBLE par defaut (valeurs sur plusieurs lignes + newline final).
# Usage:
#   Liste (tuiles/icones): python bake_sprites.py list out.h asteria::spr::TILE 16 t0.png t1.png ...
#   Grille (heros/PNJ):    python bake_sprites.py grid out.h asteria::spr::HERO 20 bas:3 haut:3 gauche:3 droite:3 --dir <dossier>
import sys, os
from PIL import Image
WRAP = 24  # valeurs hex par ligne
def bgr565(r,g,b): return ((b>>3)<<11)|((g>>2)<<5)|(r>>3)
KEY = bgr565(255,0,255)
def emit(im, size, name, L):
    im=im.convert("RGBA"); bb=im.getbbox()
    if bb: im=im.crop(bb)
    if isinstance(size,tuple): im=im.resize(size,Image.NEAREST); w,h=size
    else: h=size; w=max(1,round(im.width*h/im.height)); im=im.resize((w,h),Image.NEAREST)
    a=im.load(); v=[]
    for y in range(h):
        for x in range(w):
            r,g,b,al=a[x,y]; v.append(KEY if al<128 else bgr565(r,g,b))
    rows=[",".join(f"0x{q:04X}" for q in v[i:i+WRAP]) for i in range(0,len(v),WRAP)]
    L.append(f"static const uint16_t {name}[] = {{\n  "+",\n  ".join(rows)+"\n};")
    return w,h
def head(ns):
    parts=ns.split("::"); return parts[:-1], parts[-1]
def main():
    mode,out,fq,size = sys.argv[1],sys.argv[2],sys.argv[3],int(sys.argv[4])
    nss,table = head(fq)
    L=["#pragma once","#include <cstdint>"]
    for n in nss: L.append(f"namespace {n} {{")
    L += ["#ifndef ASTERIA_SPR_DEFS","#define ASTERIA_SPR_DEFS",
          "struct Sprite { const uint16_t* px; int16_t w; int16_t h; };",
          f"static const uint16_t KEY = 0x{KEY:04X};","#endif"]
    if mode=="list":
        metas=[]
        for i,f in enumerate(sys.argv[5:]):
            w,h=emit(Image.open(f),(size,size),f"{table}_{i}",L); metas.append((f"{table}_{i}",w,h))
        L.append(f"static const Sprite {table}[] = {{\n"+",\n".join(f"  {{{n},{w},{h}}}" for n,w,h in metas)+"\n};")
    elif mode=="grid":
        specs=[s for s in sys.argv[5:] if ":" in s]; d=sys.argv[sys.argv.index("--dir")+1]; rows=[]
        for di,spec in enumerate(specs):
            nm,cnt=spec.split(":"); cnt=int(cnt); cells=[]
            for fi in range(cnt):
                p=os.path.join(d,f"{nm}_{fi}.png"); p=p if os.path.exists(p) else os.path.join(d,f"{nm}_0.png")
                w,h=emit(Image.open(p),size,f"{table}_{di}_{fi}",L); cells.append(f"{{{table}_{di}_{fi},{w},{h}}}")
            rows.append("  {"+",".join(cells)+"}")
        L.append(f"static const Sprite {table}[{len(specs)}][{max(int(s.split(':')[1]) for s in specs)}] = {{\n"+",\n".join(rows)+"\n};")
    for _ in nss: L.append("}")
    open(out,"w").write("\n".join(L)+"\n"); print("->",out,f"({os.path.getsize(out)} o, {len(open(out).read().splitlines())} lignes)")
main()
