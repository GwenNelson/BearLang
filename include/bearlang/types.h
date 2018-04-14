#pragma once

#include <stdint.h>
#include <bearlang/uthash.h>

typedef enum bl_val_type_t {
        BL_VAL_TYPE_NULL,
        BL_VAL_TYPE_AST_LIST,
        BL_VAL_TYPE_SYMBOL,
        BL_VAL_TYPE_NUMBER,
        BL_VAL_TYPE_CONS,
        BL_VAL_TYPE_OPER_NATIVE,
	BL_VAL_TYPE_CTX,
} bl_val_type_t;

typedef struct bl_val_t bl_val_t;

struct bl_hash_t {
       char           key[32];
       bl_val_t*      val;
       UT_hash_handle hh;
};

typedef struct bl_val_t {
        bl_val_type_t type;
        union {
               struct { int64_t i_val; };
               struct { char*   s_val; };
               struct { bl_val_t* car;
                        bl_val_t* cdr; };
	       struct { struct bl_hash_t *hash_val;
	                bl_val_t* parent; };
	       struct { bl_val_t* (*code_ptr)(bl_val_t*,bl_val_t*); };
        };
} bl_val_t;

typedef struct bl_ast_node_t bl_ast_node_t;
typedef struct bl_ast_node_t {
        bl_val_t        node_val;
        bl_ast_node_t*  parent;
        bl_ast_node_t** children;
        unsigned int    child_count;
} bl_ast_node_t;

