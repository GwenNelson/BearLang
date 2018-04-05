#pragma once

#include <bearlang/types.h>
#include <bearlang/common.h>

bl_val_t* bl_ctx_new(bl_val_t* parent);
void      bl_ctx_close(bl_val_t* ctx);
bl_val_t* bl_ctx_eval(bl_val_t* ctx, bl_val_t* expr);
