#include <bearlang/common.h>
#include <bearlang/parser.h>
#include <bearlang/types.h>
#include <bearlang/utils.h>

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

bl_val_t* bl_parse_term(char* s) {
   char* c = strdup(s);
   bool is_num = true;
   bool is_sym = true;
   while(*c) {
     char the_c = *c++;
     if(the_c == '"') is_sym = false;
     if((the_c <= '0') || (the_c >= '9')) is_num=false;
   }
   if(is_num) return bl_mk_number(atoi(s));
   if(is_sym) return bl_mk_symbol(s);
   return bl_mk_null();
}
