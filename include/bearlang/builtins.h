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
