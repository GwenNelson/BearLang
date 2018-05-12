// This module provides basic file I/O


#include <bearlang/common.h>
#include <bearlang/types.h>
#include <bearlang/ctx.h>
#include <bearlang/utils.h>
#include <bearlang/error_tools.h>
#include <bearlang/list_ops.h>

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

// pre-allocated errors
bl_val_t mode_error = {
  .type=BL_VAL_TYPE_ERROR,
  .err_val = {.type   = BL_ERR_CUSTOM,
              .errmsg = "Invalid I/O mode",
              .errnum = (uint64_t)EINVAL}
};

bl_val_t perm_error = {
  .type=BL_VAL_TYPE_ERROR,
  .err_val = {.type   = BL_ERR_CUSTOM,
              .errmsg = "Access denied",
              .errnum = (uint64_t)EACCES}
};

bl_val_t generic_error = {
  .type=BL_VAL_TYPE_ERROR,
  .err_val = {.type = BL_ERR_IO}
};

bl_val_t bad_fd_error = {
  .type=BL_VAL_TYPE_ERROR,
  .err_val = {.type   = BL_ERR_CUSTOM,
              .errmsg = "Invalid file descriptor",
              .errnum = (uint64_t)EBADF}
};

// (fopen filename mode)
// filename is a string, the file to open, mode is a mode string
// returns a C pointer representing the FILE* on success, or an error on failure
bl_val_t* bl_fopen(bl_val_t* ctx, bl_val_t* params) {
     params = bl_eval_cons(ctx, params);
     bl_val_type_t expected_types[2] = {BL_VAL_TYPE_STRING,BL_VAL_TYPE_STRING};
     bl_val_t* retval = bl_errif_invalid_fixed_args(params,expected_types,2);
     if(retval != NULL) return retval;

     bl_val_t* filename_s = bl_list_first(params);
     bl_val_t* mode_s     = bl_list_second(params);

     char* filename = filename_s->s_val;
     char* mode     = mode_s->s_val;

     FILE* fd = fopen(filename,mode);
     if(fd == NULL) {
	switch(errno) {
            case EINVAL:
                 return &mode_error;
            break;
            case EACCES:
	         return &perm_error;
	    break;
	    case EROFS:
		 return &perm_error;
	    break;
	    default:
		 return &generic_error;
	    break;
	}
     }
     return bl_mk_ptr((void*)fd);
}

// (fclose FD)
// FD is a file descriptor as returned from fopen
// returns NULL on success or an error on error
bl_val_t* bl_fclose(bl_val_t* ctx, bl_val_t* params) {
     params = bl_eval_cons(ctx, params);
     bl_val_t* retval = bl_errif_invalid_firstarg(params,BL_VAL_TYPE_CPTR);
     if(retval != NULL) return retval;

     bl_val_t* bl_fd = bl_list_first(params);
     FILE* fd = (FILE*)bl_fd->c_ptr;
     int ret = fclose(fd);
     if(ret != 0) {
        switch(errno) {
            case EBADF:
                 return &bad_fd_error;
            break;
            default:
                 return &generic_error;
            break;
        }
     }
     return bl_mk_null();
}

// (fprintf FD format-string ...)
// format-string accepts the following format characters:
//    %s string (or a string representation of the value)
//    %x converts to hex representation, always has a leading 0x
//    %% literal % character
// any other format characters are ignored
bl_val_t* bl_fprintf(bl_val_t* ctx, bl_val_t* params) {
    params = bl_eval_cons(ctx, params);
    bl_val_t* retval = bl_errif_invalid_firstarg(params,BL_VAL_TYPE_CPTR);
    if(retval != NULL) return retval;
    retval = bl_errif_invalid_firstarg(params->cdr,BL_VAL_TYPE_STRING);
    if(retval != NULL) return retval;

    bl_val_t* fd         = bl_list_first(params);
    bl_val_t* format_str = bl_list_second(params);
    bl_val_t* vars       = bl_list_rest(bl_list_rest(params));

    return bl_mk_null();
}

bl_val_t* bl_mod_init(bl_val_t* ctx) {
     bl_val_t* my_ctx = bl_ctx_new(ctx);
     bl_ctx_set(my_ctx,bl_mk_symbol("fopen"),  bl_mk_native_oper(&bl_fopen));
     bl_ctx_set(my_ctx,bl_mk_symbol("fclose"), bl_mk_native_oper(&bl_fclose));
     bl_ctx_set(my_ctx,bl_mk_symbol("fprintf"),bl_mk_native_oper(&bl_fprintf));
     return my_ctx;
}
