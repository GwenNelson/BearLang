#pragma once

#include <stdint.h>

#include <bearlang/types.h>
#include <bearlang/common.h>

bl_val_t* bl_list_first(bl_val_t* L);
bl_val_t* bl_list_second(bl_val_t* L);
bl_val_t* bl_list_rest(bl_val_t* L);

bl_val_t* bl_list_append(bl_val_t* L, bl_val_t* val);
bl_val_t* bl_list_prepend(bl_val_t* L, bl_val_t* val);

uint64_t bl_list_len(bl_val_t* L);
