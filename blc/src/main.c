#include <llvm-c/Core.h>
#include <llvm-c/ExecutionEngine.h>
#include <llvm-c/Target.h>
#include <llvm-c/Analysis.h>
#include <llvm-c/BitWriter.h>

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

LLVMTypeRef bl_str_struct;
LLVMTypeRef bl_null_struct;
LLVMTypeRef bl_char_str;

static int verbose = 0;

void add_bl_init(LLVMModuleRef mod) {
     LLVMTypeRef param_types[] = {};
     LLVMTypeRef ret_type = LLVMFunctionType(LLVMVoidType(), param_types, 0, 0);
     
     LLVMAddFunction(mod, "bl_init", ret_type);
}

void add_ctx_new_std(LLVMModuleRef mod) {
     LLVMTypeRef param_types[] = {};
     LLVMTypeRef ret_type = LLVMFunctionType(bl_null_struct, param_types, 0, 0);
     
     LLVMAddFunction(mod, "bl_ctx_new_std", ret_type);
}


void add_ctx_new(LLVMModuleRef mod) {
     LLVMTypeRef param_types[] = {bl_null_struct};
     LLVMTypeRef ret_type = LLVMFunctionType(bl_null_struct, param_types, 0, 0);
     
     LLVMAddFunction(mod, "bl_ctx_new", ret_type);
}

void add_ctx_eval(LLVMModuleRef mod) {
     LLVMTypeRef param_types[] = {bl_null_struct, bl_null_struct};
     LLVMTypeRef ret_type = LLVMFunctionType(bl_null_struct, param_types, 2, 0);
     
     LLVMAddFunction(mod, "bl_ctx_eval", ret_type);
}

void add_mk_list(LLVMModuleRef mod) {
     LLVMTypeRef param_types[] =  {LLVMInt32Type()};
     LLVMTypeRef ret_type      =  LLVMFunctionType(bl_null_struct, param_types, 1, 1);
     LLVMAddFunction(mod, "bl_mk_list", ret_type);
}

void add_mk_symbol(LLVMModuleRef mod) {
     LLVMTypeRef param_types[] = {LLVMPointerType(LLVMInt8Type(),0)};
     LLVMTypeRef ret_type = LLVMFunctionType(bl_null_struct, param_types, 1, 0);
     
     LLVMAddFunction(mod, "bl_mk_symbol", ret_type);
}

void add_mk_str(LLVMModuleRef mod) {
     LLVMTypeRef param_types[] = {LLVMPointerType(LLVMInt8Type(),0)};
     LLVMTypeRef ret_type = LLVMFunctionType(bl_null_struct, param_types, 1, 0);
     
     LLVMAddFunction(mod, "bl_mk_str", ret_type);
}

LLVMModuleRef create_module(const char* mod_name) {
   LLVMModuleRef retval = LLVMModuleCreateWithName(mod_name);

   // setup the string+symbol struct types
   bl_null_struct = LLVMPointerType(LLVMStructType(NULL,0,0),0);
   bl_char_str    = LLVMPointerType(LLVMInt8Type(),0);
   LLVMTypeRef s_members[] = {LLVMInt8Type(),
	                      bl_null_struct,
	                      bl_char_str,0};

   bl_str_struct = LLVMStructType(s_members,3,0);

   // declare the parts of the BearLang API that we need to use
   add_bl_init(retval);
   add_ctx_new_std(retval);
   add_ctx_new(retval);
   add_ctx_eval(retval); 
   add_mk_list(retval);
   add_mk_symbol(retval);
   add_mk_str(retval);

   return retval;
}

bl_val_t* get_code(char* filename) {
   fprintf(stderr,"Reading from %s\n", filename);
   FILE* fd = fopen(filename,"r");
   bl_val_t* retval = bl_parse_file(filename,fd);
   fclose(fd);
   return retval;
}

void write_expr(LLVMModuleRef mod, LLVMValueRef ctx, LLVMBuilderRef builder, bl_val_t* E) {
     uint64_t expr_len = bl_list_len(E);
     LLVMValueRef *contents = GC_MALLOC(sizeof(LLVMValueRef)*(expr_len+1));
     int i=0;
     bl_val_t* L=E;
     contents[0] = LLVMConstInt(LLVMInt32Type(),expr_len,0);
     LLVMValueRef mk_params[1];
     for(i=1; i< (expr_len+1); i++) {
           mk_params[0] = LLVMBuildGlobalStringPtr(builder,L->car->s_val,"");
    	   contents[i] = LLVMBuildCall(builder,LLVMGetNamedFunction(mod,"bl_mk_symbol"),mk_params,1,"");
         L=L->cdr;
     }

     LLVMValueRef expr_list = LLVMBuildCall(builder,LLVMGetNamedFunction(mod,"bl_mk_list"),
		                                    contents,expr_len+1,"");
     LLVMValueRef eval_params[] = {ctx,expr_list};
     LLVMBuildCall(builder, LLVMGetNamedFunction(mod,"bl_ctx_eval"),
			                            eval_params,2,"");
     if(verbose) LLVMDumpModule(mod);

}

void handle_toplevel(LLVMModuleRef mod, bl_val_t* E) {
     // for now naively assume all toplevels are functions
     bl_val_t* func_name = bl_list_second(E);
     bl_val_t* func_args = bl_list_third(E);
     bl_val_t* func_body = bl_list_rest(bl_list_rest(bl_list_rest(E)));
     if(verbose) printf("Generating function %s with args %s and body %s\n", bl_ser_sexp(func_name),
		                                                 bl_ser_sexp(func_args),
								 bl_ser_sexp(func_body));
     LLVMTypeRef param_types[] = {LLVMInt32Type(), LLVMPointerType(LLVMInt8Type(),0)}; // this is for main
     LLVMTypeRef ret_type      = LLVMFunctionType(LLVMInt32Type(), param_types, 2, 0);
     LLVMValueRef main         = LLVMAddFunction(mod, "main", ret_type);

     LLVMBasicBlockRef entry = LLVMAppendBasicBlock(main, "");

     LLVMBuilderRef builder = LLVMCreateBuilder();
     LLVMPositionBuilderAtEnd(builder, entry);
     LLVMValueRef tmp = LLVMGetParam(main, 0);

     LLVMBuildCall(builder, LLVMGetNamedFunction(mod, "bl_init"), NULL,0,"");
     LLVMValueRef ctx = LLVMBuildCall(builder, LLVMGetNamedFunction(mod, "bl_ctx_new_std"), NULL, 0, "");

     bl_val_t* L = func_body;
     while(L->car != NULL) {
     LLVMPositionBuilderAtEnd(builder, entry);
       	     write_expr(mod,ctx,builder,L->car);
      L = L->cdr;
      if(L==NULL) break;
     }

 

     LLVMBuildRet(builder, tmp);
     LLVMDisposeBuilder(builder);
}

void compile_file(char* input_filename, char* output_filename) {
     bl_val_t* code = get_code(input_filename);
     LLVMModuleRef mod = create_module(input_filename);
     bl_val_t* L = code;

     while(L->car != NULL) {
      if(L->car != NULL) handle_toplevel(mod,L->car);
      L = L->cdr;
      if(L==NULL) break;
     }

     char *error = NULL;
     LLVMVerifyModule(mod, LLVMAbortProcessAction, &error);
     LLVMDisposeMessage(error);

     // Write out bitcode to file
     if (LLVMWriteBitcodeToFile(mod, output_filename) != 0) {
         fprintf(stderr, "error writing bitcode to file!\n");
     }


}


static char* exe_name;

#define ERR_BAD_PARAMS 1
#define ERR_NOT_OPEN   2


void print_usage() {
     fprintf(stderr,"usage: %s -i filename -o output_file [-v]\n", exe_name);
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

    while ((c = getopt(argc, argv, "i:o:v")) != -1) {
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

