#pragma once
namespace asteria {
void flag_set(const char* name); bool flag_get(const char* name);
void quest_start(int i); void quest_complete(int i); void quest_set_obj(int i,int o);
int  quest_status(int i);  int  quest_obj(int i);   // status: 0 aucune, 1 en cours, 2 terminee
void state_reset();
}
