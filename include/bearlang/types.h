#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <bearlang/uthash.h>

#define BL_LONGEST_LIST 0xFFFFFFFFFFFFFFFF

typedef enum bl_val_type_t {
        BL_VAL_TYPE_NULL,        // The None or NULL type
        BL_VAL_TYPE_ERROR,       // Error / exception - if this is returned anywhere, something went wrong
	BL_VAL_TYPE_AST_LIST,    // A list from the AST
        BL_VAL_TYPE_SYMBOL,      // A BearLang symbol
        BL_VAL_TYPE_NUMBER,      // A number
        BL_VAL_TYPE_STRING,      // A string
	BL_VAL_TYPE_CONS,        // A cons cell (or a list)
        BL_VAL_TYPE_OPER_NATIVE, // A native-code operator
	BL_VAL_TYPE_FUNC_BL,     // A BearLang-code function (uncompiled)
        BL_VAL_TYPE_FUNC_NATIVE, // A native-code function
	BL_VAL_TYPE_CTX,         // A context
} bl_val_type_t;

typedef enum bl_err_type_t {
	BL_ERR_PARSE,              // Failed to parse an s-expression
	BL_ERR_INSUFFICIENT_ARGS,  // Not enough arguments were provided
        BL_ERR_TOOMANY_ARGS,       // Too many arguments were provided
} bl_err_type_t;

typedef struct bl_err_t {
	bl_err_type_t type;
	union {
                // BL_ERR_INSUFFICIENT_ARGS
		struct {uint64_t min_args;
			uint64_t max_args;
			uint64_t provided_args; };
	};
} bl_err_t;

typedef struct bl_val_t bl_val_t;

struct bl_hash_t {
       char           key[32];
       bl_val_t*      val;
       UT_hash_handle hh;
};

typedef struct bl_val_t {
        bl_val_type_t type;
        union {
               struct { bl_err_t err_val; }; // BL_VAL_TYPE_ERROR
 	       struct { int64_t i_val; }; // BL_VAL_TYPE_NUMBER
               struct { char*   s_val; }; // BL_VAL_TYPE_SYMBOL | BL_VAL_TYPE_STRING
               struct { bl_val_t* car;    // BL_VAL_TYPE_CONS
                        bl_val_t* cdr; };
	       struct { struct bl_hash_t *hash_val; // BL_VAL_TYPE_CTX
	                bl_val_t* parent; };
	       struct { bl_val_t* (*code_ptr)(bl_val_t*,bl_val_t*); }; // BL_VAL_TYPE_OPER_NATIVE
	       struct { bl_val_t* bl_func_ptr;    // BL_VAL_TYPE_FUNC_BL_RAW
		        bl_val_t* bl_funcargs_ptr; };
	       struct { bl_val_t* (*func_ptr)(bl_val_t*,bl_val_t*); }; // BL_VAL_TYPE_FUNC_NATIVE
        };
} bl_val_t;

typedef struct bl_ast_node_t bl_ast_node_t;
typedef struct bl_ast_node_t {
        bl_val_t        node_val;
        bl_ast_node_t*  parent;
        bl_ast_node_t** children;
        unsigned int    child_count;
} bl_ast_node_t;

