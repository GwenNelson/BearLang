#include <bearlang/common.h>
#include <bearlang/list_ops.h>

#include <stdio.h>

bl_val_t* bl_list_first(bl_val_t* L) {
   if(L==NULL) return NULL;
//   if(L->type == BL_VAL_TYPE_ERROR) return L;
   return L->car;
}

bl_val_t* bl_list_second(bl_val_t* L) {
   if(L==NULL) return NULL;
//   if(L->type == BL_VAL_TYPE_ERROR) return L;
   return bl_list_first(bl_list_rest(L));
}

bl_val_t* bl_list_rest(bl_val_t* L) {
   if(L==NULL) return NULL;
//   if(L->type == BL_VAL_TYPE_ERROR) return L;
   return L->cdr;
}

bl_val_t* bl_list_third(bl_val_t* L) {
   return bl_list_second(bl_list_rest(L));
}

bl_val_t* bl_list_prepend(bl_val_t* L, bl_val_t* val) {
   bl_val_t* retval = (bl_val_t*)GC_MALLOC(sizeof(bl_val_t));
   retval->type = BL_VAL_TYPE_CONS;
   retval->car  = val;
   retval->cdr  = L;
   return retval;
}

bl_val_t* bl_list_append(bl_val_t* L, bl_val_t* val) {
   if(L==NULL) {
      return bl_list_prepend(L, val);
   }
   
   bl_val_t* retval = L;
   while(L->cdr != NULL) {
      L = L->cdr;
   }
   L->cdr = (bl_val_t*)GC_MALLOC(sizeof(bl_val_t));
   L->cdr->type = BL_VAL_TYPE_CONS;
   L->cdr->car  = val;
   L->cdr->cdr  = NULL;
   return retval;
}

uint64_t bl_list_len(bl_val_t* L) {
   uint64_t retval = 0;
   if(L==NULL) return 0;
   bl_val_t* i = L;
   while(L->cdr != NULL) {
      if(L->car != NULL) retval++;
      L = L->cdr;
   }
   if(L->car != NULL) retval++;
   return retval;
}
