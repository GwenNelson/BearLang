#include <bearlang/common.h>
#include <bearlang/types.h>
#include <bearlang/sexp.h>
#include <bearlang/ctx.h>
#include <bearlang/error_tools.h>
#include <bearlang/utils.h>
#include <bearlang/list_ops.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>

static int verbose   = 0;
static int build_lib = 0;

static char* exe_name;

#define ERR_BAD_PARAMS 1
#define ERR_NOT_OPEN   2


void print_usage() {
     fprintf(stderr,"usage: %s -i filename -o output_file [-v] [-l]\n", exe_name);
}

void compile_expr(bl_val_t* expr, FILE* outfd) {
     size_t expr_len = bl_list_len(expr);
     fprintf(outfd,"retval = bl_ctx_eval(ctx,bl_mk_list(%d,",expr_len);
     bl_val_t* L = expr;
     for(; L != NULL; L=L->cdr) {
         if(L->car->type == BL_VAL_TYPE_STRING) {
	    fprintf(outfd,"bl_mk_str(\"%s\")",L->car->s_val);
         } else {
            fprintf(outfd,"bl_parse_sexp(\"%s\")",bl_ser_sexp(L->car));
	 }
	 if(L->cdr != NULL) fprintf(outfd,",");
     }
     fprintf(outfd,");");
}

void compile_fun(bl_val_t* fun_expr, FILE* outfd) {
     bl_val_t* params   = bl_list_rest(fun_expr);
     bl_val_t* fun_name = bl_list_first(params);
     bl_val_t* args     = bl_list_second(params);
     bl_val_t* body     = bl_list_rest(bl_list_rest(params));
     if(body->car->type == BL_VAL_TYPE_DOCSTRING) body=body->cdr;
     fprintf(outfd,"bl_val_t* bl_%s(bl_val_t* ctx, bl_val_t* params) {",fun_name->s_val);
     fprintf(outfd,"params = bl_ctx_eval(ctx,params);");
     fprintf(outfd,"bl_val_t* retval = NULL;");
     bl_val_t* L = body;
     for(; L != NULL; L=L->cdr) {
         compile_expr(L->car,outfd);
     }
     fprintf(outfd,"}");
}

void compile_file(char* infile, char* outfile) {
     FILE* outfd = fopen(outfile,"w");
     FILE* infd  = fopen(infile,"r");
     bl_val_t* parsed_file = bl_parse_file(infile,infd);
     if(parsed_file->type == BL_VAL_TYPE_ERROR) {
        char* errmsg = bl_errmsg(parsed_file);
        fprintf(stderr,"Error occurred in %s: %s\n", infile, errmsg);
        fclose(outfd);
	fclose(infd);
	return;
     }
     fprintf(outfd,"#include <bearlang/bearlang.h>\n");
     bl_val_t* L = parsed_file;
     bl_val_t* set_oper = bl_mk_symbol("=");
     bl_val_t* fun_oper = bl_mk_symbol("fun");
     for(; L != NULL; L=L->cdr) {
         bl_val_t* expr = L->car;
         if(expr->car==set_oper) {
	 } else if (expr->car==fun_oper) {
           compile_fun(expr,outfd);
	 } else {
           fprintf(stderr,"Can not compile toplevel expression %s\n", bl_ser_sexp(expr));
	   fclose(outfd);
	   fclose(infd);
	   return;
	 }
     }
     fclose(outfd);
     fclose(infd);
}

int main(int argc, char** argv) {
    GC_INIT();
    bl_init();

    extern char *optarg;
    extern int optind;

    exe_name = strdup(argv[0]);

    char* input_filename     = NULL;
    char* output_filename    = NULL;

    int c = 0;

    while ((c = getopt(argc, argv, "i:o:lv")) != -1) {
       switch(c) {
          case 'i':
             input_filename = strdup(optarg);
          break;

          case 'o':
             output_filename = strdup(optarg);
          break;

          case 'v':
             verbose = 1;
          break;

	  case 'l':
	     build_lib = 1;
	  break;

          case '?':
             print_usage();
             exit(ERR_BAD_PARAMS);
          break;

       }
    }

    if(input_filename == NULL) {
      print_usage();
      exit(ERR_BAD_PARAMS);
    }

    if(output_filename == NULL) {
      print_usage();
      exit(ERR_BAD_PARAMS);
    }

    compile_file(input_filename,output_filename);

    return 0;
}

