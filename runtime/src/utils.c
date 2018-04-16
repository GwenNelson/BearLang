#include <bearlang/utils.h>

#include <string.h>
#include <stdio.h>

bl_val_t* bl_mk_val(bl_val_type_t type) {
   bl_val_t* retval = (bl_val_t*)GC_MALLOC(sizeof(bl_val_t));
   retval->type     = type;
   return retval;
}

bl_val_t* bl_mk_symbol(char* sym) {
   bl_val_t* retval = bl_mk_val(BL_VAL_TYPE_SYMBOL);
   size_t count     = strlen(sym)*sizeof(char);
   retval->s_val    = (char*)GC_MALLOC(count);
   snprintf(retval->s_val,count,"%s",sym);
   return retval;
}

bl_val_t* bl_mk_number(uint64_t n) {
   bl_val_t* retval = bl_mk_val(BL_VAL_TYPE_NUMBER);
   retval->i_val    = n;
   return retval;
}

bl_val_t* bl_mk_str(char* s) {
   bl_val_t* retval = bl_mk_val(BL_VAL_TYPE_SYMBOL);
   size_t count     = strlen(s)*sizeof(char);
   retval->s_val    = (char*)GC_MALLOC(count);
   snprintf(retval->s_val,count,"%s",s);
   return retval;
}

bl_val_t* bl_mk_native_oper(void* func_ptr) {
   bl_val_t* retval = bl_mk_val(BL_VAL_TYPE_OPER_NATIVE);
   retval->code_ptr = func_ptr;
   return retval;
}

bl_val_t* bl_mk_bool(bool b) {
   bl_val_t* retval = bl_mk_val(BL_VAL_TYPE_BOOL);
   if(b) {
     retval->i_val = 1;
   } else {
     retval->i_val = 0;
   }
   return retval;
}
