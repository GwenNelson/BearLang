#pragma once

#include <stdint.h>

typedef enum bl_val_type_t {
        BL_VAL_TYPE_LIST,
        BL_VAL_TYPE_SYMBOL,
        BL_VAL_TYPE_INT,
} bl_val_type_t;

typedef union {
        struct { int64_t i_val; };
} bl_val_t;

typedef struct bl_ast_node_t bl_ast_node_t;
typedef struct bl_ast_node_t {
        bl_val_type_t   node_type;
        bl_val_t        node_val;
        bl_ast_node_t*  parent;
        bl_ast_node_t** children;
        unsigned int    child_count;
} bl_ast_node_t;



