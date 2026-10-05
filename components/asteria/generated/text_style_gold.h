// GENERE PAR tools/genfonts.py — style doré
#pragma once
#include <cstdint>
namespace asteria { namespace ui {
static constexpr uint8_t GOLD_ROWS=14;
struct RGB{uint8_t r,g,b;};
static const RGB GOLD_OUTLINE={48,24,8};
static const RGB GOLD_GRAD[GOLD_ROWS]={
  {255,246,182},
  {255,238,159},
  {255,230,136},
  {255,221,113},
  {255,213,91},
  {252,202,80},
  {250,191,69},
  {248,181,58},
  {245,170,47},
  {240,158,40},
  {232,146,36},
  {224,133,32},
  {216,121,28},
  {208,108,24},
};
} }
