#include <bearlang/utils.h>
#include <bearlang/sexp.h>
#include <bearlang/mpc.h>
#include <bearlang/ctx.h>
#include <bearlang/sexp.h>

#include <string.h>
#include <stdio.h>
#include <stdbool.h>

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
   if(b==true) {
     retval->i_val = 1;
   } else {
     retval->i_val = 0;
   }
   return retval;
}

extern mpc_parser_t* Lispy;
bl_val_t* bl_eval_file(bl_val_t* ctx, char* filename, FILE* fd) {
  // TODO - move some of this crap into sexp.c 
  mpc_result_t r;
  bl_ast_node_t* file_ast = NULL;
  bl_val_t* retval = NULL;
  int i=0;

  if(mpc_parse_file(filename, fd, Lispy, &r)) {
     mpc_ast_t* mpc_ast = r.output;
     retval = bl_ctx_eval(ctx, bl_read_ast(mpc_to_bl(mpc_ast)));
     mpc_ast_delete(r.output);
  } else {
     mpc_err_print(r.error);
     mpc_err_delete(r.error);
  }
  return retval;
}
