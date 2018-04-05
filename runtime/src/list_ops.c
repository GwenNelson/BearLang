#include <bearlang/common.h>
#include <bearlang/list_ops.h>

#include <stdio.h>

bl_val_t* bl_list_first(bl_val_t* L) {
   if(L==NULL) return NULL;
   return L->car;
}

bl_val_t* bl_list_second(bl_val_t* L) {
   if(L==NULL) return NULL;
   return bl_list_first(bl_list_rest(L));
}

bl_val_t* bl_list_rest(bl_val_t* L) {
   if(L==NULL) return NULL;
   return L->cdr;
}

bl_val_t* bl_list_prepend(bl_val_t* L, bl_val_t* val) {
   bl_val_t* retval = (bl_val_t*)GC_MALLOC(sizeof(bl_val_t));
   retval->type = BL_VAL_TYPE_CONS;
   retval->car  = val;
   retval->cdr  = L;
   return retval;
}

bl_val_t* bl_list_append(bl_val_t* L, bl_val_t* val) {
   bl_val_t* retval = L;
   while(bl_list_rest(L) != NULL) {
     L = bl_list_rest(L);
     if(bl_list_rest(L)==NULL) {
         L->cdr = (bl_val_t*)GC_MALLOC(sizeof(bl_val_t));
         L->cdr->type = BL_VAL_TYPE_CONS;
         L->cdr->car  = val;
         L->cdr->cdr  = NULL;
         return L;
      }
      L = bl_list_rest(L);
   }
   return retval;
}
