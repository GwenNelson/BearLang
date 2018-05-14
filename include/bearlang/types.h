#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <bearlang/uthash.h>
#include <gmp.h>

#undef uthash_malloc
#undef uthash_free

#define uthash_malloc(sz) GC_MALLOC(sz)
#define uthash_free(ptr,sz) 



#define BL_LONGEST_LIST 0xFFFFFFFFFFFFFFFF

typedef enum bl_val_type_t {
        BL_VAL_TYPE_NULL=1,         // The None or NULL type
        BL_VAL_TYPE_ERROR=2,        // Error / exception - if this is returned anywhere, something went wrong
        BL_VAL_TYPE_SYMBOL=3,       // A BearLang symbol
        BL_VAL_TYPE_NUMBER=4,       // A number
	BL_VAL_TYPE_BOOL=5,         // A boolean value
        BL_VAL_TYPE_STRING=6,       // A string
	BL_VAL_TYPE_CONS=7,         // A cons cell (or a list)
        BL_VAL_TYPE_OPER_NATIVE=8,  // A native-code operator
	BL_VAL_TYPE_OPER_BL=9,      // A BearLang-code operator
	BL_VAL_TYPE_FUNC_BL=10,     // A BearLang-code function (uncompiled)
        BL_VAL_TYPE_FUNC_NATIVE=11, // A native-code function
	BL_VAL_TYPE_CTX=12,         // A context
	BL_VAL_TYPE_CPTR=13,        // For use by extension modules
	BL_VAL_TYPE_ANY=14,         // Any type (only used for error handling etc)
        BL_VAL_TYPE_LIST_END=15,    // used by the parser
	BL_VAL_TYPE_OPER_DO=16,     // special form
	BL_VAL_TYPE_OPER_IF=17,     // special form
	BL_VAL_TYPE_OPER_WHILE=18,  // special form
	BL_VAL_TYPE_DOCSTRING=19,   // docstrings
} bl_val_type_t;

typedef enum bl_err_type_t {
	BL_ERR_ANY=0,		     // Matches ANY error type
	BL_ERR_UNKNOWN=1,            // Generic / unknown error
	BL_ERR_PARSE=2,              // Failed to parse an s-expression
	BL_ERR_INSUFFICIENT_ARGS=3,  // Not enough arguments were provided
        BL_ERR_TOOMANY_ARGS=4,       // Too many arguments were provided
	BL_ERR_INVALID_ARGTYPE=5,    // Invalid argument type(s) was/were provided
	BL_ERR_SYMBOL_NOTFOUND=6,    // Attempted to evaluate an expression containing a symbol that wasn't found
	BL_ERR_DIVIDE_BY_ZERO=7,     // Attempted to divide by zero
	BL_ERR_CUSTOM=8,             // Custom error type - contains a human-readable string and a numeric error code
	BL_ERR_IO=9,                 // Generic I/O error
	BL_ERR_MODULE_NOTFOUND=10,   // Failed to find a module
} bl_err_type_t;

// TODO: add all builtins as tokens to the lexer
typedef enum bl_token_type_t {
	BL_TOKEN_LPAREN     = 1,
	BL_TOKEN_RPAREN     = 2,
	BL_TOKEN_STRING     = 3,
	BL_TOKEN_INTEGER    = 4,
	BL_TOKEN_SYMBOL     = 5,
	BL_TOKEN_IF         = 6,
	BL_TOKEN_DO         = 7,
	BL_TOKEN_WHILE      = 8,
	BL_TOKEN_DOCSTRING  = 9,
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
	char* errmsg;      // for BL_ERR_CUSTOM
	uint64_t errnum;    // for BL_ERR_CUSTOM
} bl_err_t;

struct bl_hash_t {
       bl_val_t*      key;
       bl_val_t*      val;
       UT_hash_handle hh;
};

typedef struct bl_val_t {
        bl_val_type_t type;
	bl_val_t* eval_last; // used by bl_eval_cons
	bl_val_t* docstr;    // docstring for this value
	union {

		// BL_VAL_TYPE_ERROR
		struct { bl_err_t err_val; };

		// BL_VAL_TYPE_BOOL
		struct { bool b_val; };

		// BL_VAL_TYPE_NUMBER
		struct { int64_t fix_int;
			 mpz_t i_val;
	                 mpf_t f_val;
			 bool is_float;	};

		// BL_VAL_TYPE_SYMBOL | BL_VAL_TYPE_STRING
		struct { char*   s_val;
	                 uint64_t sym_id; };

		// BL_VAL_TYPE_CONS
		struct { bl_val_t* car;
                         bl_val_t* cdr;	};

		// BL_VAL_TYPE_CTX
		struct { void* custom_ctx_data; // for use by custom get function in C extension modules
			 bl_val_t** keys;
			 bl_val_t** vals;
			 size_t vals_count;
			 bl_val_t* parent;
	                 bl_val_t* secondary;
			 bl_val_t* (*ctx_get)(bl_val_t*,bl_val_t*); // custom get function, can override default, signature is: bl_val_t* ctx_get(bl_val_t* ctx,bl_val_t* sym);
	                 bool write_to_parent; };

		// BL_VAL_TYPE_CPTR
		struct { void* c_ptr; };

		// BL_VAL_TYPE_OPER_NATIVE
		struct { bl_val_t* (*code_ptr)(bl_val_t*,bl_val_t*); };

		// BL_VAL_TYPE_OPER_BL
		struct { bl_val_t* bl_oper_ptr; 
		         bl_val_t* bl_operargs_ptr; };

		// BL_VAL_TYPE_FUNC_BL
		struct { bl_val_t* bl_func_ptr;
		         bl_val_t* bl_funcargs_ptr;
	                 bl_val_t* lexical_closure;
			 bl_val_t* inner_closure;
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

