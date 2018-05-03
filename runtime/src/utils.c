#include <bearlang/utils.h>
#include <bearlang/sexp.h>
#include <bearlang/ctx.h>
#include <bearlang/sexp.h>
#include <bearlang/list_ops.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include <gmp.h>



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

struct sym_hash_t {
   char key[32];
   bl_val_t* sym;
   UT_hash_handle hh;
};

struct sym_hash_t* symbol_table = NULL;
uint16_t last_sym_id = 0;

bl_val_t* bl_mk_symbol(char* sym) {
   struct sym_hash_t* symobj = NULL;
   bl_val_t* retval = NULL;
   HASH_FIND_STR(symbol_table, sym, symobj);
   if(!symobj) {
      retval           = bl_mk_val(BL_VAL_TYPE_SYMBOL);
      size_t count     = strlen(sym)*sizeof(char)+1;
      retval->s_val    = (char*)GC_MALLOC_ATOMIC(count);
      retval->sym_id   = last_sym_id;
      last_sym_id++;
      snprintf(retval->s_val,count,"%s",sym);
      symobj = (struct sym_hash_t*)GC_MALLOC(sizeof(struct sym_hash_t));
      snprintf(symobj->key,32,"%s", sym);
      symobj->sym = retval;
      HASH_ADD_STR(symbol_table, key, symobj);
   } else {
      retval = symobj->sym;
   }
   return retval;
}

bl_val_t* bl_mk_integer(char* s) {
   bl_val_t* retval = bl_mk_val(BL_VAL_TYPE_NUMBER);
/*   if(mpz_init_set_str(retval->i_val,s,10)==-1) {
      fprintf(stderr,"Error in gmp!\n");
   }*/
   retval->fix_int = atoll(s);
   retval->is_float = false;
   return retval;
}

bl_val_t* bl_mk_float(char* s) {
   bl_val_t* retval = bl_mk_val_atomic(BL_VAL_TYPE_NUMBER);
   mpf_init_set_str(retval->f_val,s,10);
   retval->is_float = true;
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
	             .b_val = true};
bl_val_t false_val = {.type = BL_VAL_TYPE_BOOL,
	              .b_val = false};

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
