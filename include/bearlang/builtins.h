#pragma once

#include <bearlang/types.h>
#include <bearlang/common.h>

bl_val_t* bl_oper_add   (bl_val_t* ctx, bl_val_t* params);
bl_val_t* bl_oper_sub   (bl_val_t* ctx, bl_val_t* params);
bl_val_t* bl_oper_mult  (bl_val_t* ctx, bl_val_t* params);
bl_val_t* bl_oper_div   (bl_val_t* ctx, bl_val_t* params);

bl_val_t* bl_oper_fun   (bl_val_t* ctx, bl_val_t* params);
bl_val_t* bl_oper_fn    (bl_val_t* ctx, bl_val_t* params);
bl_val_t* bl_oper_set   (bl_val_t* ctx, bl_val_t* params);
bl_val_t* bl_oper_print (bl_val_t* ctx, bl_val_t* params);

bl_val_t* bl_oper_eq    (bl_val_t* ctx, bl_val_t* params);
bl_val_t* bl_oper_if    (bl_val_t* ctx, bl_val_t* params);

bl_val_t* bl_oper_do    (bl_val_t* ctx, bl_val_t* params);

bl_val_t* bl_oper_and   (bl_val_t* ctx, bl_val_t* params);
bl_val_t* bl_oper_not   (bl_val_t* ctx, bl_val_t* params);
bl_val_t* bl_oper_or    (bl_val_t* ctx, bl_val_t* params);
bl_val_t* bl_oper_xor   (bl_val_t* ctx, bl_val_t* params);

bl_val_t* bl_oper_first  (bl_val_t* ctx, bl_val_t* params);
bl_val_t* bl_oper_second (bl_val_t* ctx, bl_val_t* params);
bl_val_t* bl_oper_third  (bl_val_t* ctx, bl_val_t* params);
bl_val_t* bl_oper_rest   (bl_val_t* ctx, bl_val_t* params);
