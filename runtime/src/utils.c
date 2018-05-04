#include <bearlang/utils.h>
#include <bearlang/sexp.h>
#include <bearlang/ctx.h>
#include <bearlang/sexp.h>
#include <bearlang/list_ops.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include <gmp.h>


#define POOL_DEFAULT_SIZE 100000

bl_val_t val_pool_static[POOL_DEFAULT_SIZE];

bl_val_t* val_pool = &val_pool_static;
uint64_t last_alloc = 0;
uint64_t val_pool_size = POOL_DEFAULT_SIZE;


bl_val_t* bl_mk_val(bl_val_type_t type) { // LCOV_EXCL_LINE

   if(last_alloc >= val_pool_size) {
      val_pool = (bl_val_t*)GC_MALLOC(sizeof(bl_val_t)*val_pool_size);
      last_alloc = 0;
   }

   bl_val_t* retval = &(val_pool[last_alloc++]);
   retval->type = type;
   return retval;
}

bl_val_t null_val = {.type = BL_VAL_TYPE_NULL};
bl_val_t* bl_mk_null() { // LCOV_EXCL_LINE

   return &null_val;
}

struct sym_hash_t {
   char key[32];
   bl_val_t* sym;
   UT_hash_handle hh;
};

struct sym_hash_t* symbol_table = NULL;
uint64_t last_sym_id = 0;

bl_val_t* bl_mk_symbol(char* sym) { // LCOV_EXCL_LINE

   struct sym_hash_t* symobj = NULL;
   bl_val_t* retval = NULL;
//LCOV_EXCL_START
   HASH_FIND_STR(symbol_table, sym, symobj);
//LCOV_EXCL_STOP
   if(!symobj) {
      retval           = bl_mk_val(BL_VAL_TYPE_SYMBOL);
      size_t count     = strlen(sym)*sizeof(char)+1;
      retval->s_val    = (char*)GC_MALLOC_ATOMIC(count);
      retval->sym_id   = last_sym_id;
      last_sym_id++;
      snprintf(retval->s_val,count+1,"%s",sym);
      symobj = (struct sym_hash_t*)GC_MALLOC(sizeof(struct sym_hash_t));
      snprintf(symobj->key,32,"%s", retval->s_val);
      symobj->sym = retval;
//LCOV_EXCL_START
      HASH_ADD_STR(symbol_table, key, symobj);
//LCOV_EXCL_STOP
   } else {
      retval = symobj->sym;
   }
   return retval;
}

bl_val_t* bl_mk_integer(char* s) { // LCOV_EXCL_LINE

   bl_val_t* retval = bl_mk_val(BL_VAL_TYPE_NUMBER);
/*   if(mpz_init_set_str(retval->i_val,s,10)==-1) {
      fprintf(stderr,"Error in gmp!\n");
   }*/
   retval->fix_int = atoll(s);
   retval->is_float = false;
   return retval;
}


bl_val_t* bl_mk_str(char* s) { // LCOV_EXCL_LINE

   bl_val_t* retval = bl_mk_val(BL_VAL_TYPE_STRING);
   size_t count     = strlen(s)*sizeof(char)+1;
   retval->s_val    = (char*)GC_MALLOC_ATOMIC(count);
   snprintf(retval->s_val,count,"%s",s);
   return retval;
}

bl_val_t* bl_mk_native_oper(void* func_ptr) { // LCOV_EXCL_LINE

   bl_val_t* retval = bl_mk_val(BL_VAL_TYPE_OPER_NATIVE);
   retval->code_ptr = func_ptr;
   return retval;
}

bl_val_t true_val = {.type = BL_VAL_TYPE_BOOL,
	             .b_val = true};
bl_val_t false_val = {.type = BL_VAL_TYPE_BOOL,
	              .b_val = false};

bl_val_t* bl_mk_bool(bool b) { // LCOV_EXCL_LINE

   if(b) return &true_val;
   return &false_val;
}

bl_val_t* bl_mk_list(size_t count,...) { // LCOV_EXCL_LINE


   bl_val_t* retval = NULL;
   if(count==0) {
      retval = bl_mk_val(BL_VAL_TYPE_CONS);
      retval->car=NULL;
      retval->cdr=NULL;
      return retval;
   }

   va_list ap;
   bl_val_t* E;
   va_start(ap,count);

   int i=0;
   for (i=0; i < count; i++ ) {
      retval = bl_list_prepend(retval,va_arg(ap,bl_val_t*)); // LCOV_EXCL_LINE
   }
   va_end(ap);
   return bl_list_reverse(retval);
}

bl_val_t* bl_eval_file(bl_val_t* ctx, char* filename, FILE* fd) { // LCOV_EXCL_LINE

   bl_val_t*     sexp = bl_parse_file(filename, fd);
   bl_val_t*   retval = bl_ctx_eval(ctx, sexp);
   return retval;
}
