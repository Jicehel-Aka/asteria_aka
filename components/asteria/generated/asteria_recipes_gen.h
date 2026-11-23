// GENERE + ENRICHI : 7 recettes utiles (Forge station=0, Alchimie portable station=-1)
#pragma once
#include <cstdint>
#include "asteria_gen.h"
namespace asteria { namespace data {
struct Ingredient{int16_t item;uint8_t qty;};
struct Recipe{const char* id;int16_t station;const Ingredient* inputs;uint8_t input_n;int16_t out_item;uint8_t out_qty;};
static const Ingredient REC0_IN[]={{12,1},{11,1}};   // Lingot rouille + Charbon -> Lingot de fer
static const Ingredient REC1_IN[]={{30,1},{0,1}};    // Lingot de fer + Epee usee -> Epee de fer
static const Ingredient REC2_IN[]={{33,2},{34,1}};   // 2 Herbe + Champignon -> Petite potion de soin
static const Ingredient REC3_IN[]={{4,2},{25,1}};    // 2 Petite potion + Miel sauvage -> Grande potion de soin
static const Ingredient REC4_IN[]={{30,1},{35,2}};   // Lingot de fer + 2 Bois -> 5 Fleches renforcees
static const Ingredient REC5_IN[]={{30,2},{35,2}};   // 2 Lingot de fer + 2 Bois -> Bouclier de voyage
static const Ingredient REC6_IN[]={{30,1},{10,1}};   // Lingot de fer + Huile lanterne -> Lanterne renforcee
static constexpr uint8_t RECIPE_COUNT=7;
static const Recipe RECIPES[RECIPE_COUNT]={
  {"REC_0001",0,REC0_IN,2,30,1},
  {"REC_0002",0,REC1_IN,2,31,1},
  {"REC_0003",-1,REC2_IN,2,4,1},
  {"REC_0004",-1,REC3_IN,2,32,1},
  {"REC_0005",0,REC4_IN,2,26,5},
  {"REC_0006",0,REC5_IN,2,22,1},
  {"REC_0007",0,REC6_IN,2,18,1},
};
} }
