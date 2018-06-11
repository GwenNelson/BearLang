// This module provides basic file I/O

// TODO - implement a standard stream data type in the core runtime and integrate it here

#include <bearlang/common.h>
#include <bearlang/types.h>
#include <bearlang/ctx.h>
#include <bearlang/utils.h>
#include <bearlang/error_tools.h>
#include <bearlang/list_ops.h>
#include <bearlang/sexp.h>

#include <readline/readline.h>

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

bl_val_t eof_error = {
  .type=BL_VAL_TYPE_ERROR,
  .err_val = {.type   = BL_ERR_CUSTOM,
              .errmsg = "File is closed",
              .errnum = (uint64_t)EPIPE,}
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

// utility function to convert arbitrary values into hexdump
char* bl_hexdump(bl_val_t* V) {
    char* retval = NULL;
    if(V->type == BL_VAL_TYPE_NUMBER) {
       retval = (char*)GC_MALLOC_ATOMIC(sizeof(char)*24);
       snprintf(retval,20,"0x%llx", V->fix_int);
       return retval;
    }
    char* buf = bl_ser_naked_sexp(V);
    size_t buflen = (strlen(buf)*2)+4;
    retval = GC_MALLOC_ATOMIC(sizeof(char)*buflen);
    snprintf(buf,2,"%s","0x");
    int i=0;
    for(i=0; i<strlen(buf); i++) {
        snprintf(buf,buflen,"%s%x",buf,(uint8_t)buf[i]);
    }
    return retval;
}

// (fprintf FD format-string ...)
// format-string accepts the following format characters:
//    %s string (or a string representation of the value)
//    %x converts to hex representation, always has a leading 0x
//    %% literal % character
// any other format characters will be ignored
// if not enough arguments are provided, (null) or 0x00 will be printed in place of the format character
bl_val_t* bl_fprintf(bl_val_t* ctx, bl_val_t* params) {
    params = bl_eval_cons(ctx, params);
    bl_val_t* retval = bl_errif_invalid_firstarg(params,BL_VAL_TYPE_CPTR);
    if(retval != NULL) return retval;
    retval = bl_errif_invalid_firstarg(params->cdr,BL_VAL_TYPE_STRING);
    if(retval != NULL) return retval;

    bl_val_t* bl_fd      = bl_list_first(params);
    bl_val_t* format_str = bl_list_second(params);
    bl_val_t* vars       = bl_list_rest(bl_list_rest(params));

    FILE* fd = (FILE*)bl_fd->c_ptr;
    char*  s = format_str->s_val;

    bl_val_t* var_iter = vars;

    bool format_sym;
    int i;
    char* buf = NULL;;
    for(i=0; s[i] != '\0'; i++) {
        switch(s[i]) {
	   case '%':
             if(format_sym) {
               if(fputc('%',fd) == EOF) return &generic_error;
               format_sym = false;
             } else {
               format_sym = true;
             }
	   break;
	   case 's':
	      if(format_sym) {
                 buf = bl_ser_naked_sexp(bl_list_first(var_iter));
                 if(strlen(buf)==0) buf="(null)";
                 if(fprintf(fd,"%s",buf) == EOF) return &generic_error;
		 format_sym = false;
                 var_iter = bl_list_rest(var_iter);
	      } else {
                 if(fputc('s',fd) == EOF) return &generic_error;
              }
              break;
	   case 'x':
	      if(format_sym) {
                 buf = bl_hexdump(bl_list_first(var_iter));
                 if(fprintf(fd,"%s",buf) == EOF) return &generic_error;
                 format_sym = false;
                 var_iter = bl_list_rest(var_iter);
	      } else {
                 if(fputc('s',fd) == EOF) return &generic_error;
              }
              break;
	   default:
             if(fputc(s[i],fd) == EOF) return &generic_error;
           break;
	}
    }

    return bl_mk_null();
}

// (fgets FD maxlen)
// FD is a file descriptor as returned by fopen, maxlen is a number specifying the size of the buffer to use
// returns a string on success, or an error
bl_val_t* bl_fgets(bl_val_t* ctx, bl_val_t* params) {
     params = bl_ctx_eval(ctx,params);
     bl_val_type_t expected_types[2] = {BL_VAL_TYPE_CPTR,BL_VAL_TYPE_NUMBER};
     bl_val_t* retval = bl_errif_invalid_fixed_args(params,expected_types,2);
     if(retval != NULL) return retval;

     bl_val_t* bl_fd     = bl_list_first(params);
     bl_val_t* bl_maxlen = bl_list_second(params);

     FILE* fd = (FILE*)(bl_fd->c_ptr);
     int maxlen = (int)bl_maxlen->fix_int;

     if(feof(fd) != 0) return &eof_error; 

     char* retval_s = GC_MALLOC_ATOMIC(sizeof(char)*maxlen);
     if(fgets(retval_s,maxlen,fd)==NULL) {
        switch(errno) {
            case EBADF:
                 return &bad_fd_error;
            break;
            default:
                 return &generic_error;
            break;
        }
     }
     return bl_mk_str(retval_s);
}

// (readline prompt)
// simple interface to the readline library, if prompt is provided then it is used as the prompt
// on success, returns the line that was read, if readline() returned NULL then an error is returned
bl_val_t* bl_readline(bl_val_t* ctx, bl_val_t* params) {
     char* prompt = "";
     if(bl_list_len(params)>0) {
        prompt = bl_ser_naked_sexp(bl_list_first(params));
     }
     char* line = readline(prompt);
     if(line != NULL) {
        bl_val_t* retval = bl_mk_str(line);
	free(line);
	return retval;
     }
     return &generic_error;
}

// (popen command mode)
// simple wrapper around popen(3)
// on success, returns a FILE* pointer
bl_val_t* bl_popen(bl_val_t* ctx, bl_val_t* params) {
     params = bl_ctx_eval(ctx,params);
     bl_val_t* first   = bl_list_first(params);
     bl_val_t* second  = bl_list_second(params);
     char* cmd  = first->s_val;
     char* mode = second->s_val;
     FILE* retval = popen((const char*)cmd,(const char*)mode);
     return bl_mk_ptr((void*)retval);
}

// (pclose stream)
// simple wrapper around pclose(3) - must be used instead of fclose
bl_val_t* bl_pclose(bl_val_t* ctx, bl_val_t* params) {
     params = bl_ctx_eval(ctx,params);
     bl_val_t* first   = bl_list_first(params);
     pclose(first->c_ptr);
     return bl_mk_null();
}

bl_val_t* bl_mod_init(bl_val_t* ctx) {
     bl_val_t* my_ctx = bl_ctx_new(ctx);
     bl_ctx_set(my_ctx,bl_mk_symbol("fopen"),   bl_mk_native_oper(&bl_fopen));
     bl_ctx_set(my_ctx,bl_mk_symbol("fclose"),  bl_mk_native_oper(&bl_fclose));
     bl_ctx_set(my_ctx,bl_mk_symbol("fprintf"), bl_mk_native_oper(&bl_fprintf));
     bl_ctx_set(my_ctx,bl_mk_symbol("fgets"),   bl_mk_native_oper(&bl_fgets));
     bl_ctx_set(my_ctx,bl_mk_symbol("readline"),bl_mk_native_oper(&bl_readline));
     bl_ctx_set(my_ctx,bl_mk_symbol("popen"),   bl_mk_native_oper(&bl_popen));
     bl_ctx_set(my_ctx,bl_mk_symbol("pclose"),  bl_mk_native_oper(&bl_pclose));

     bl_ctx_set(my_ctx,bl_mk_symbol("STDIN"),   bl_mk_ptr((void*)stdin));
     bl_ctx_set(my_ctx,bl_mk_symbol("STDOUT"),  bl_mk_ptr((void*)stdout));
     return my_ctx;
}
