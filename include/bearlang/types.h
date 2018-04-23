#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <bearlang/uthash.h>

#undef uthash_malloc
#undef uthash_free

#define uthash_malloc(sz) GC_MALLOC(sz)
#define uthash_free(ptr,sz) GC_FREE(ptr)

#define BL_LONGEST_LIST 0xFFFFFFFFFFFFFFFF

typedef enum bl_val_type_t {
        BL_VAL_TYPE_NULL,        // The None or NULL type
        BL_VAL_TYPE_ERROR,       // Error / exception - if this is returned anywhere, something went wrong
	BL_VAL_TYPE_AST_LIST,    // A list from the AST
        BL_VAL_TYPE_SYMBOL,      // A BearLang symbol
        BL_VAL_TYPE_NUMBER,      // A number
	BL_VAL_TYPE_BOOL,        // A boolean value
        BL_VAL_TYPE_STRING,      // A string
	BL_VAL_TYPE_CONS,        // A cons cell (or a list)
        BL_VAL_TYPE_OPER_NATIVE, // A native-code operator
	BL_VAL_TYPE_OPER_BL,     // A BearLang-code operator
	BL_VAL_TYPE_FUNC_BL,     // A BearLang-code function (uncompiled)
        BL_VAL_TYPE_FUNC_NATIVE, // A native-code function
	BL_VAL_TYPE_CTX,         // A context
	BL_VAL_TYPE_ANY,         // Any type (only used for error handling etc)
        BL_VAL_TYPE_LIST_END,    // used by the parser
	BL_VAL_TYPE_OPER_DO,     // special form
	BL_VAL_TYPE_OPER_IF,     // special form
} bl_val_type_t;

typedef enum bl_err_type_t {
	BL_ERR_UNKNOWN=1,            // Generic / unknown error
	BL_ERR_PARSE=2,              // Failed to parse an s-expression
	BL_ERR_INSUFFICIENT_ARGS=3,  // Not enough arguments were provided
        BL_ERR_TOOMANY_ARGS=4,       // Too many arguments were provided
	BL_ERR_INVALID_ARGTYPE=5,    // Invalid argument type(s) was/were provided
	BL_ERR_SYMBOL_NOTFOUND=6,
} bl_err_type_t;

typedef enum bl_token_type_t {
	BL_TOKEN_LPAREN  = 1,
	BL_TOKEN_RPAREN  = 2,
	BL_TOKEN_STRING  = 3,
	BL_TOKEN_FLOAT   = 4,
	BL_TOKEN_INTEGER = 5,
	BL_TOKEN_SYMBOL  = 6,
} bl_token_type_t;

typedef struct bl_val_t bl_val_t;

typedef struct bl_err_t {
	bl_err_type_t type;

	uint64_t min_args;
	uint64_t max_args;
	uint64_t provided_args;

	bl_val_type_t* expected_types; // expected argument types
        bl_val_type_t* provided_types; // actually provided argument types (as bl_val_type_t array)
	
	char* symbol_name; // only set if relevant to the error
} bl_err_t;

struct bl_hash_t {
       char           key[32];
       bl_val_t*      val;
       UT_hash_handle hh;
};

typedef struct bl_val_t {
        bl_val_type_t type;
	bl_val_t* eval_last; // used by bl_eval_cons
	union {

		// BL_VAL_TYPE_ERROR
		struct { bl_err_t err_val; };

		// BL_VAL_TYPE_NUMBER | BL_VAL_TYPE_BOOL
		struct { int64_t i_val;
	                 float   f_val;	};

		// BL_VAL_TYPE_SYMBOL | BL_VAL_TYPE_STRING
		struct { char*   s_val; };

		// BL_VAL_TYPE_CONS
		struct { bl_val_t* car;
                         bl_val_t* cdr; };

		// BL_VAL_TYPE_CTX
		struct { struct bl_hash_t *hash_val;
	                 bl_val_t* parent;
	                 bl_val_t* secondary;
	                 bool write_to_parent; };

		// BL_VAL_TYPE_OPER_NATIVE
		struct { bl_val_t* (*code_ptr)(bl_val_t*,bl_val_t*); };

		// BL_VAL_TYPE_OPER_BL
		struct { bl_val_t* bl_oper_ptr; 
		         bl_val_t* bl_operargs_ptr; };

		// BL_VAL_TYPE_FUNC_BL
		struct { bl_val_t* bl_func_ptr;
		         bl_val_t* bl_funcargs_ptr;
	                 bl_val_t* lexical_closure;
	       		 bl_val_t* sym;	};

		// BL_VAL_TYPE_FUNC_NATIVE
		struct { bl_val_t* (*func_ptr)(bl_val_t*,bl_val_t*); };
        };
} bl_val_t;

typedef struct bl_ast_node_t bl_ast_node_t;
typedef struct bl_ast_node_t {
        bl_val_t        node_val;
        bl_ast_node_t*  parent;
        bl_ast_node_t** children;
        unsigned int    child_count;
} bl_ast_node_t;

