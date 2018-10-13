#pragma once

#include <bearlang/types.h>
#include <bearlang/common.h>

bl_val_t* bl_eval(bl_val_t* ctx, bl_val_t* expr);
bl_val_t* bl_eval_cons(bl_val_t* ctx, bl_val_t* expr);

