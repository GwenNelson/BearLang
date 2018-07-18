#pragma once

#include <bearlang/common.h>
#include <bearlang/types.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdarg.h>

void  bl_init_jit();
void* bl_jit_func(bl_val_t* f);

