#pragma once

#include <bearlang/common.h>

char* safe_strcat(char* a, char* b);          // concatenates a+b
char* join_str(bl_val_t* L, char* sep);       // joins together a list of strings using the seperator in sep
char* str_replace(char* s, char* a, char* b); // returns a copy of s with every instance of a replaced with b
bl_val_t* split_str(char* s, char* sep);      // splits s into a list, using seperator sep
