#include <bearlang/utils.h>
#include <bearlang/sexp.h>
#include <bearlang/ctx.h>
#include <bearlang/sexp.h>
#include <bearlang/list_ops.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

bl_val_t* bl_mk_val(bl_val_type_t type) {
   bl_val_t* retval = (bl_val_t*)GC_MALLOC(sizeof(bl_val_t));
   retval->type     = type;
   return retval;
}

bl_val_t* bl_mk_val_atomic(bl_val_type_t type) {
   bl_val_t* retval = (bl_val_t*)GC_MALLOC_ATOMIC(sizeof(bl_val_t));
   retval->type     = type;
   return retval;
}

bl_val_t null_val = {.type = BL_VAL_TYPE_NULL};
bl_val_t* bl_mk_null() {
   return &null_val;
}

bl_val_t* bl_mk_symbol(char* sym) {
   bl_val_t* retval = bl_mk_val(BL_VAL_TYPE_SYMBOL);
   size_t count     = strlen(sym)*sizeof(char)+1;
   retval->s_val    = (char*)GC_MALLOC_ATOMIC(count);
   snprintf(retval->s_val,count,"%s",sym);
   return retval;
}

bl_val_t* bl_mk_number(uint64_t n) {
   bl_val_t* retval = bl_mk_val_atomic(BL_VAL_TYPE_NUMBER);
   retval->i_val    = n;
   return retval;
}

bl_val_t* bl_mk_float(float f) {
   bl_val_t* retval = bl_mk_val_atomic(BL_VAL_TYPE_NUMBER);
   retval->f_val    = f;
   return retval;
}

bl_val_t* bl_mk_str(char* s) {
   bl_val_t* retval = bl_mk_val(BL_VAL_TYPE_STRING);
   size_t count     = strlen(s)*sizeof(char)+1;
   retval->s_val    = (char*)GC_MALLOC_ATOMIC(count);
   snprintf(retval->s_val,count,"%s",s);
   return retval;
}

bl_val_t* bl_mk_native_oper(void* func_ptr) {
   bl_val_t* retval = bl_mk_val(BL_VAL_TYPE_OPER_NATIVE);
   retval->code_ptr = func_ptr;
   return retval;
}

bl_val_t true_val = {.type = BL_VAL_TYPE_BOOL,
	             .i_val = 1};
bl_val_t false_val = {.type = BL_VAL_TYPE_BOOL,
	              .i_val = 0};

bl_val_t* bl_mk_bool(bool b) {
   if(b) return &true_val;
   return &false_val;
}

bl_val_t* bl_mk_list(size_t count,...) {
   bl_val_t* retval = NULL;

   va_list ap;
   bl_val_t* E;
   va_start(ap,count);

   int i=0;
   for (i=0; i < count; i++ ) {
      retval = bl_list_append(retval,va_arg(ap,bl_val_t*));
   }
   return retval;
}

bl_val_t* bl_eval_file(bl_val_t* ctx, char* filename, FILE* fd) {
   bl_val_t*     sexp = bl_parse_file(filename, fd);
   bl_val_t*   retval = bl_ctx_eval(ctx, sexp);
   return retval;
}
