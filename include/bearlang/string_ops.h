#pragma once

#include <bearlang/common.h>

char* safe_strcat(char* a, char* b);
char* join_str(bl_val_t* L, char* sep);
bl_val_t* split_str(char* s, char* sep);

