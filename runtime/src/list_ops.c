#include <bearlang/common.h>
#include <bearlang/list_ops.h>
#include <bearlang/utils.h>

#include <stdio.h>

// TODO - add type errors here

bl_val_t* bl_list_first(bl_val_t* L) { // LCOV_EXCL_LINE

   if(L==NULL) return NULL;
//   if(L->type == BL_VAL_TYPE_ERROR) return L;
   return L->car;
}

bl_val_t* bl_list_second(bl_val_t* L) { // LCOV_EXCL_LINE

   if(L==NULL) return NULL;
//   if(L->type == BL_VAL_TYPE_ERROR) return L;
   return bl_list_first(bl_list_rest(L));
}

bl_val_t* bl_list_rest(bl_val_t* L) { // LCOV_EXCL_LINE

   if(L==NULL) return NULL;
//   if(L->type == BL_VAL_TYPE_ERROR) return L;
   return L->cdr;
}

bl_val_t* bl_list_third(bl_val_t* L) { // LCOV_EXCL_LINE

   return bl_list_second(bl_list_rest(L));
}

bl_val_t* bl_list_prepend(bl_val_t* L, bl_val_t* val) { // LCOV_EXCL_LINE

   bl_val_t* retval = bl_mk_val(BL_VAL_TYPE_CONS);
   retval->car  = val;
   retval->cdr  = L;
   return retval;
}

bl_val_t* bl_list_last(bl_val_t* L) { // LCOV_EXCL_LINE
   if(L==NULL) {
      return L;
   }
   
   while(L->cdr != NULL) {
      L = L->cdr;
   }
   return L->car;
}

bl_val_t* bl_list_append(bl_val_t* L, bl_val_t* val) { // LCOV_EXCL_LINE

   if(L==NULL) {
      return bl_list_prepend(L, val);
   }
   
   bl_val_t* retval = L;
   while(L->cdr != NULL) {
      L = L->cdr;
   }
   L->cdr = bl_mk_val(BL_VAL_TYPE_CONS);
   L->cdr->car  = val;
   L->cdr->cdr  = NULL;
   return retval;
}

uint64_t bl_list_len(bl_val_t* L) { // LCOV_EXCL_LINE
   if(L==NULL) return 0;
   if(L->type == BL_VAL_TYPE_NULL) return 0;
   if(L->car==NULL) return 0;
   uint64_t retval = 0;

   bl_val_t* i = L;
   for(i=L; i != NULL; i=i->cdr) {
       retval++;
   }
   return retval;
}

bl_val_t* bl_list_reverse(bl_val_t* L) { // LCOV_EXCL_LINE

   bl_val_t* retval = NULL;
   while(L != NULL) {
      retval = bl_list_prepend(retval,L->car);
      L = L->cdr;
   }
   return retval;
}
