#include <gc.h>
#include <stdio.h>
#include <stdlib.h>
#include <libgen.h>
#include <errno.h>

#include <bearlang/common.h>
#include <bearlang/types.h>
#include <bearlang/sexp.h>
#include <bearlang/ctx.h>
#include <bearlang/error_tools.h>
#include <bearlang/utils.h>
#include <bearlang/list_ops.h>
#include <bl_build_config.h>

#include <readline/readline.h>

#include <replxx.h>

static char *line_read = (char *)NULL;

const char* history_file = ".bl_history";
static Replxx* replxx;

char *
rl_gets ()
{
    do {
    	line_read = replxx_input(replxx, ">>> ");
    } while((line_read==NULL) && (errno == EAGAIN));
    if(line_read==NULL) {
       printf("\n");
       
    } else {
      replxx_history_add( replxx, line_read );
    }

    return line_read;
}

void run_file(char* filename, int argc, char** argv) {
     bl_val_t* STDLIB_CTX = bl_ctx_new_std();
     bl_val_t* FILE_CTX   = bl_ctx_new(STDLIB_CTX);

     bl_ctx_set(FILE_CTX, bl_mk_symbol("*MAINFILE*"), bl_mk_str(basename(filename)));
     bl_ctx_set(FILE_CTX, bl_mk_symbol("*FILENAME*"), bl_mk_str(basename(filename)));

     int i=0;
     bl_val_t* argv_cons = NULL;
     for(i=0; i<argc; i++) {
         argv_cons = bl_list_append(argv_cons, bl_mk_str(argv[i]));
     }
     bl_ctx_set(FILE_CTX, bl_mk_symbol("*ARGV*"), argv_cons);

     FILE* fd = fopen(filename,"r");

     bl_val_t* retval = bl_eval_file(FILE_CTX, filename, fd);
     if(retval->type == BL_VAL_TYPE_ERROR) {
        char* errmsg = bl_errmsg(retval);
	fprintf(stderr,"Error occurred in %s: %s\n", filename, errmsg);
     }
     fclose(fd);
}

bl_val_t* quit_cmd(bl_val_t* ctx, bl_val_t* params) {
    printf("Goodbye!\n");
    replxx_history_save(replxx, history_file);
    replxx_end(replxx);
    exit(0);
}

void hint_hook(char const* prefix, int bp,replxx_hints* lc, ReplxxColor* c, void* ud) {
     bl_val_t* ctx = (bl_val_t*)ud;
     size_t len = strlen(prefix);
     int i=0;
     if (len > bp) {
        for(i=0; i <= ctx->vals_count; i++) {
          if(ctx->vals[i] != NULL) {
      		if (strncmp(prefix + bp, ctx->keys[i]->s_val, strlen(prefix) - bp) == 0) {
				replxx_add_hint(lc, ctx->keys[i]->s_val + len - bp);
		}
	  }
	 }

        if(ctx->parent != NULL) {
	for(i=0; i <= ctx->parent->vals_count; i++) {
          if(ctx->parent->vals[i] != NULL) {
      		if (strncmp(prefix + bp, ctx->parent->keys[i]->s_val, strlen(prefix) - bp) == 0) {
			
				if(ctx->parent->vals[i]->docstr != NULL) {
					replxx_add_hint(lc, ctx->parent->vals[i]->docstr->s_val + len);
				} else {
					replxx_add_hint(lc, ctx->parent->keys[i]->s_val + len - bp);
				}
		}
	  }
	 }
        }

    }
}

void completion_hook(char const* prefix, int bp, replxx_hints* lc, ReplxxColor* c, void* ud) {
     bl_val_t* ctx = (bl_val_t*)ud;
     size_t len = strlen(prefix);
     int i=0;
     for(i=0; i <= ctx->vals_count; i++) {
         if(ctx->vals[i] != NULL) {
            if (strncmp(prefix + bp, ctx->keys[i]->s_val, strlen(prefix) - bp) == 0) {
			replxx_add_completion(lc, ctx->keys[i]->s_val);
		}
	 }
     }
     if(ctx->parent != NULL) {
        for(i=0; i <= ctx->parent->vals_count; i++) {
   	     if(ctx->parent->vals[i] != NULL) {
                if (strncmp(prefix + bp, ctx->parent->keys[i]->s_val, strlen(prefix) - bp) == 0) {
			replxx_add_completion(lc, ctx->parent->keys[i]->s_val);
		}
              }
         }
     }
}

int main(int argc, char** argv) {
    bl_init();

    if(argc>=2) {
       char* filename = argv[1];
       run_file(filename,argc-1,argv+1);
       return 0;
    }

    printf("BearLang Version %s\n\n",BL_VERSION);

    replxx = replxx_init();
    replxx_install_window_change_handler(replxx);


    replxx_history_load(replxx,history_file);


    bl_val_t* STDLIB_CTX = bl_ctx_new_std();
    bl_val_t* REPL_CTX   = bl_ctx_new(STDLIB_CTX);

    bl_ctx_set(REPL_CTX,bl_mk_symbol("*MAINFILE*"), bl_mk_str(""));
    bl_ctx_set(REPL_CTX,bl_mk_symbol("*FILENAME*"), bl_mk_str(""));
    bl_ctx_set(REPL_CTX,bl_mk_symbol("quit"), bl_mk_native_oper(&quit_cmd));

    bl_eval(REPL_CTX,bl_parse_sexp("(import bldoc)"));
    bl_eval(REPL_CTX,bl_parse_sexp("(using bldoc::help)"));


    replxx_set_hint_callback( replxx, hint_hook, REPL_CTX );
    replxx_set_completion_callback( replxx, completion_hook, REPL_CTX);

    char* errmsg;
    for(;;) {
        char* input_line = rl_gets();
        if(input_line) {
		bl_val_t* expr    = bl_parse_sexp(input_line);
		bl_val_t*      result = bl_eval(REPL_CTX, expr);
		switch(result->type) {
	           case BL_VAL_TYPE_NULL:
	              printf("\n");
		   break;
	           case BL_VAL_TYPE_ERROR:
	              errmsg = bl_errmsg(result);
	       	      printf("Error occurred:\n%s\n", errmsg);
		   break;
		   default:
	              printf("%s\n",bl_ser_sexp(result));
		   break;
		}
	} else {
		printf("\nType (quit) to quit\n");
	}
	GC_gcollect();
    }
    return 0;
}
