#pragma once

#include <bearlang/types.h>
#include <bearlang/common.h>

bl_val_t* bl_oper_add   (bl_val_t* ctx, bl_val_t* params);
bl_val_t* bl_oper_sub   (bl_val_t* ctx, bl_val_t* params);
bl_val_t* bl_oper_mult  (bl_val_t* ctx, bl_val_t* params);
bl_val_t* bl_oper_div   (bl_val_t* ctx, bl_val_t* params);
bl_val_t* bl_oper_mod   (bl_val_t* ctx, bl_val_t* params);

bl_val_t* bl_oper_fun   (bl_val_t* ctx, bl_val_t* params);
bl_val_t* bl_oper_fn    (bl_val_t* ctx, bl_val_t* params);
bl_val_t* bl_oper_oper  (bl_val_t* ctx, bl_val_t* params);
bl_val_t* bl_oper_set   (bl_val_t* ctx, bl_val_t* params);
bl_val_t* bl_oper_print (bl_val_t* ctx, bl_val_t* params);

bl_val_t* bl_oper_serexp (bl_val_t* ctx, bl_val_t* params);
bl_val_t* bl_oper_parse  (bl_val_t* ctx, bl_val_t* params);
bl_val_t* bl_oper_eval   (bl_val_t* ctx, bl_val_t* params);

bl_val_t* bl_oper_isset (bl_val_t* ctx, bl_val_t* params);

bl_val_t* bl_oper_include (bl_val_t* ctx, bl_val_t* params);
bl_val_t* bl_oper_import  (bl_val_t* ctx, bl_val_t* params);

bl_val_t* bl_oper_eq    (bl_val_t* ctx, bl_val_t* params);
bl_val_t* bl_oper_lt    (bl_val_t* ctx, bl_val_t* params);
bl_val_t* bl_oper_gt    (bl_val_t* ctx, bl_val_t* params);

bl_val_t* bl_oper_map   (bl_val_t* ctx, bl_val_t* params);

bl_val_t* bl_oper_and   (bl_val_t* ctx, bl_val_t* params);
bl_val_t* bl_oper_not   (bl_val_t* ctx, bl_val_t* params);
bl_val_t* bl_oper_or    (bl_val_t* ctx, bl_val_t* params);
bl_val_t* bl_oper_xor   (bl_val_t* ctx, bl_val_t* params);

bl_val_t* bl_oper_first  (bl_val_t* ctx, bl_val_t* params);
bl_val_t* bl_oper_second (bl_val_t* ctx, bl_val_t* params);
bl_val_t* bl_oper_third  (bl_val_t* ctx, bl_val_t* params);
bl_val_t* bl_oper_rest   (bl_val_t* ctx, bl_val_t* params);

bl_val_t* bl_oper_append  (bl_val_t* ctx, bl_val_t* params);
bl_val_t* bl_oper_prepend (bl_val_t* ctx, bl_val_t* params);

bl_val_t* bl_oper_reverse (bl_val_t* ctx, bl_val_t* params);

bl_val_t* bl_oper_inc (bl_val_t* ctx, bl_val_t* params);
bl_val_t* bl_oper_dec (bl_val_t* ctx, bl_val_t* params);
