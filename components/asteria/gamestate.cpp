#include "gamestate.h"
#include <cstring>
namespace asteria {
static const char* g_flags[64]; static int g_nf=0;
static unsigned char g_qst[64]={0}, g_qobj[64]={0};
void state_reset(){ g_nf=0; for(int i=0;i<64;i++){g_qst[i]=0;g_qobj[i]=0;} }
bool flag_get(const char* n){ if(!n||!n[0])return false; for(int i=0;i<g_nf;i++) if(!strcmp(g_flags[i],n))return true; return false; }
void flag_set(const char* n){ if(!n||!n[0]||flag_get(n))return; if(g_nf<64) g_flags[g_nf++]=n; }
void quest_start(int i){ if(i>=0&&i<64&&g_qst[i]==0){ g_qst[i]=1; g_qobj[i]=1; } }
void quest_complete(int i){ if(i>=0&&i<64) g_qst[i]=2; }
void quest_set_obj(int i,int o){ if(i>=0&&i<64) g_qobj[i]=(unsigned char)o; }
int  quest_status(int i){ return (i>=0&&i<64)?g_qst[i]:0; }
int  quest_obj(int i){ return (i>=0&&i<64)?g_qobj[i]:0; }
}
