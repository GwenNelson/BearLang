#pragma once

#include <bearlang/types.h>
#include <bearlang/common.h>
#include <stdbool.h>

bl_val_t* bl_ctx_new_std();
bl_val_t* bl_ctx_new(bl_val_t* parent);
void      bl_ctx_close(bl_val_t* ctx);

bl_val_t* bl_ctx_eval(bl_val_t* ctx, bl_val_t* expr);
bl_val_t* bl_eval_cons(bl_val_t* ctx, bl_val_t* expr, bool build_new_list);

bl_val_t* bl_ctx_get(bl_val_t* ctx, bl_val_t* key);
bl_val_t* bl_ctx_set(bl_val_t* ctx, bl_val_t* key, bl_val_t* val);
