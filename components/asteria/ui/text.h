#pragma once
#include "platform/gb_port.h"
namespace asteria { namespace ui {
void text(int x,int y,const char* s, gb::Color c);   // police corps (14px)
void text_big(int x,int y,const char* s, gb::Color c);// police titre (18px)
void gold(int x,int y,const char* s, bool bright=true);// corps, dégradé or + contour
int  text_w(const char* s);                           // largeur mesurée (corps)
}}
