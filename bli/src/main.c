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

void run_file(char* filename, int argc, char** argv) {
     bl_val_t* STDLIB_CTX = bl_ctx_new_std();
     bl_val_t* FILE_CTX   = bl_ctx_new(STDLIB_CTX);

     bl_ctx_set(FILE_CTX, "*MAINFILE*", bl_mk_str(basename(filename)));

     int i=0;
     bl_val_t* argv_cons = NULL;
     for(i=0; i<argc; i++) {
         argv_cons = bl_list_append(argv_cons, bl_mk_str(argv[i]));
     }
     bl_ctx_set(FILE_CTX, "*ARGV*", argv_cons);

     FILE* fd = fopen(filename,"r");

     bl_val_t* retval = bl_eval_file(FILE_CTX, filename, fd);
     if(retval->type == BL_VAL_TYPE_ERROR) {
        char* errmsg = bl_errmsg(retval);
	fprintf(stderr,"Error occurred in %s: %s\n", filename, errmsg);
     }
     fclose(fd);
}

int main(int argc, char** argv) {
    GC_INIT();
    bl_init();

    if(argc>=2) {
       char* filename = argv[1];
       run_file(filename,argc-1,argv+1);
       return 0;
    }

    printf("BearLang Version 0.WHATEVER\n\n"); // TODO: add versioning

    bl_val_t* STDLIB_CTX = bl_ctx_new_std();
    bl_val_t* REPL_CTX   = bl_ctx_new(STDLIB_CTX);

    for(;;) {
        char* input_line = readline(">>> ");
	bl_val_t* expr    = bl_parse_sexp(input_line);
	bl_val_t*      result = bl_ctx_eval(REPL_CTX, expr);
        char*          errmsg = "";
	switch(result->type) {
           case BL_VAL_TYPE_NULL:
              printf("None\n");
	   break;
           case BL_VAL_TYPE_ERROR:
              errmsg = bl_errmsg(result);
       	      printf("Error occurred:\n%s\n", errmsg);
	   break;
	   default:
              printf("%s\n",bl_ser_sexp(result));
	   break;
	}
	GC_FREE(expr);
    }
    return 0;
}
