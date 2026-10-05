#include "i18n.h"
namespace asteria { namespace i18n {
static int g_lang = FR;
void set_lang(int l){ if(l>=0&&l<LANG_COUNT) g_lang=l; }
int  get_lang(){ return g_lang; }
void next_lang(){ g_lang=(g_lang+1)%LANG_COUNT; }
const char* tr(int id){ if(id<0||id>=STR_COUNT) return "?"; return STR[id][g_lang]; }
}}
