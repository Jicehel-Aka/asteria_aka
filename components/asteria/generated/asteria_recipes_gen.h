// GENERE PAR tools/bake.py
#pragma once
#include <cstdint>
#include "asteria_gen.h"
namespace asteria { namespace data {
struct Ingredient{int16_t item;uint8_t qty;};
struct Recipe{const char* id;int16_t station;const Ingredient* inputs;uint8_t input_n;int16_t out_item;uint8_t out_qty;};
static const Ingredient REC0_IN[]={{12,1},{11,1}};
static const Ingredient REC1_IN[]={{30,1},{0,1}};
static const Ingredient REC2_IN[]={{33,2},{34,1}};
static const Ingredient REC3_IN[]={{4,2},{25,1}};
static constexpr uint8_t RECIPE_COUNT=4;
static const Recipe RECIPES[RECIPE_COUNT]={
  {"REC_0001",0,REC0_IN,2,30,1},
  {"REC_0002",0,REC1_IN,2,31,1},
  {"REC_0003",-1,REC2_IN,2,4,1},
  {"REC_0004",-1,REC3_IN,2,32,1},
};
} }
