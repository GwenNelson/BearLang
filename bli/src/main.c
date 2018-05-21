#include <gc.h>
#include <stdio.h>
#include <stdlib.h>
#include <libgen.h>

#include <bearlang/common.h>
#include <bearlang/types.h>
#include <bearlang/sexp.h>
#include <bearlang/ctx.h>
#include <bearlang/error_tools.h>
#include <bearlang/utils.h>
#include <bearlang/list_ops.h>

#include <readline/readline.h>

// shamelessly ripped straight from the readline documentation
static char *line_read = (char *)NULL;

char *
rl_gets ()
{
  if (line_read)
    {
      free (line_read);
      line_read = (char *)NULL;
    }

  line_read = readline (">>> ");

  if (line_read && *line_read)
    add_history (line_read);

  return (line_read);
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
    exit(0);
}

int main(int argc, char** argv) {
    bl_init();

    if(argc>=2) {
       char* filename = argv[1];
       run_file(filename,argc-1,argv+1);
       return 0;
    }

    printf("BearLang Version 0.WHATEVER\n\n"); // TODO: add versioning

    bl_val_t* STDLIB_CTX = bl_ctx_new_std();
    bl_val_t* REPL_CTX   = bl_ctx_new(STDLIB_CTX);

    bl_ctx_set(REPL_CTX,bl_mk_symbol("*MAINFILE*"), bl_mk_str(""));
    bl_ctx_set(REPL_CTX,bl_mk_symbol("*FILENAME*"), bl_mk_str(""));
    bl_ctx_set(REPL_CTX,bl_mk_symbol("quit"), bl_mk_native_oper(&quit_cmd));

    for(;;) {
        char* input_line = rl_gets();
        if(input_line) {
		bl_val_t* expr    = bl_parse_sexp(input_line);
		bl_val_t*      result = bl_ctx_eval(REPL_CTX, expr);
	        char*          errmsg = "";
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
