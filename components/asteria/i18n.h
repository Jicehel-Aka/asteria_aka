#pragma once
#include "generated/asteria_lang.h"
namespace asteria { namespace i18n {
void set_lang(int l); int get_lang(); void next_lang();
const char* tr(int id);
}}
