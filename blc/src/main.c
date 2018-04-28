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

LLVMTypeRef bl_str_struct;
LLVMTypeRef bl_null_struct;
LLVMTypeRef bl_char_str;

void add_ctx_eval(LLVMModuleRef mod) {
     LLVMTypeRef param_types[] = {LLVMPointerType(LLVMStructType(NULL,0,0),0),
                                  LLVMPointerType(LLVMStructType(NULL,0,0),0)};
     LLVMTypeRef ret_type = LLVMFunctionType(LLVMPointerType(LLVMStructType(NULL,0,0),0), param_types, 2, 0);
     
     LLVMAddFunction(mod, "bl_ctx_eval", ret_type);
}

void add_mk_list(LLVMModuleRef mod) {
     LLVMTypeRef param_types[] =  {LLVMPointerType(bl_str_struct,0)};
     LLVMTypeRef ret_type      =  LLVMFunctionType(LLVMInt64Type(), param_types, 1, 0);
     printf("mk_list: %s\n", LLVMPrintValueToString(LLVMAddFunction(mod, "bl_mk_list", ret_type)));
}

void add_mk_symbol(LLVMModuleRef mod) {
     LLVMTypeRef param_types[] = {LLVMArrayType(LLVMInt8Type(),2)};
     LLVMTypeRef ret_type = LLVMFunctionType(LLVMPointerType(bl_null_struct,0), param_types, 1, 0);
     
     LLVMAddFunction(mod, "bl_mk_symbol", ret_type);
}

void add_mk_str(LLVMModuleRef mod) {
     LLVMTypeRef param_types[] = {LLVMPointerType(LLVMInt8Type(),0)};
     LLVMTypeRef ret_type = LLVMFunctionType(LLVMPointerType(LLVMStructType(NULL,0,0),0), param_types, 1, 0);
     
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

LLVMValueRef llvmGenLocalStringVar(LLVMModuleRef mod, const char* data, int len)
{
  LLVMValueRef glob = LLVMAddGlobal(mod, LLVMArrayType(LLVMInt8Type(), len), "");

  // set as internal linkage and constant
  LLVMSetLinkage(glob, LLVMInternalLinkage);
  LLVMSetGlobalConstant(glob, 1);

  // Initialize with string:
  LLVMSetInitializer(glob, LLVMConstString(data, len, 1));



  return glob;
}

void write_expr(LLVMModuleRef mod, LLVMBuilderRef builder, bl_val_t* E) {
     uint64_t expr_len = bl_list_len(E);
     LLVMValueRef *contents = GC_MALLOC(sizeof(LLVMValueRef)*expr_len);
     int i=0;
     bl_val_t* L=E;
     LLVMValueRef single_contents[3];
     for(i=0; i< expr_len; i++) {
         single_contents[0] = LLVMConstInt(LLVMInt8Type(), (unsigned long long) L->car->type,0); // write type value
         single_contents[1] = LLVMConstNull(bl_null_struct);
	 single_contents[2] = LLVMBuildGlobalStringPtr(builder,L->car->s_val,"");
	 contents[i] = LLVMConstNamedStruct(bl_str_struct,single_contents,3);
         L=L->cdr;
     }

     LLVMValueRef glob = LLVMAddGlobal(mod,LLVMArrayType(bl_str_struct, expr_len),"");
     LLVMSetLinkage(glob, LLVMInternalLinkage);
     LLVMSetGlobalConstant(glob, 1);
     LLVMSetInitializer(glob, LLVMConstArray(bl_str_struct,contents,expr_len));
//     LLVMValueRef index = LLVMBuildLoad(builder, glob, "");
     LLVMValueRef mk_list_args[] = {glob};
     printf("\n\n");
     printf("%s:\n", bl_ser_sexp(E));
     LLVMDumpValue(mk_list_args[0]);
     printf("\n\n");
     LLVMDumpModule(mod);
     LLVMValueRef expr_list = LLVMBuildCall(builder,LLVMGetNamedFunction(mod,"bl_mk_list"),
		                                    mk_list_args,1,"");

}

void handle_toplevel(LLVMModuleRef mod, bl_val_t* E) {
     // for now naively assume all toplevels are functions
     bl_val_t* func_name = bl_list_second(E);
     bl_val_t* func_args = bl_list_third(E);
     bl_val_t* func_body = bl_list_rest(bl_list_rest(bl_list_rest(E)));
     printf("Generating function %s with args %s and body %s\n", bl_ser_sexp(func_name),
		                                                 bl_ser_sexp(func_args),
								 bl_ser_sexp(func_body));
     LLVMTypeRef param_types[] = {LLVMInt32Type(), LLVMPointerType(LLVMInt8Type(),0)}; // this is for main
     LLVMTypeRef ret_type      = LLVMFunctionType(LLVMInt32Type(), param_types, 2, 0);
     LLVMValueRef main         = LLVMAddFunction(mod, "main", ret_type);

     LLVMBasicBlockRef entry = LLVMAppendBasicBlock(main, "entry");

     LLVMBuilderRef builder = LLVMCreateBuilder();
     LLVMPositionBuilderAtEnd(builder, entry);
     LLVMValueRef tmp = LLVMGetParam(main, 0);

     bl_val_t* L = func_body;
     while(L->car != NULL) {
      write_expr(mod,builder,L->car);
      L = L->cdr;
      if(L==NULL) break;
     }

 

     LLVMBuildRet(builder, tmp);
     LLVMDisposeBuilder(builder);
}


int main(int argc, char** argv) {
    GC_INIT();
    bl_init();

    char* filename = argv[1];

    bl_val_t* code = get_code(filename);
    LLVMModuleRef mod = create_module(filename);
    bl_val_t* L = code;

    char output_filename[1024];
    snprintf(output_filename,1024,"%s.bc", filename);


    while(L->car != NULL) {
      if(L->car != NULL) handle_toplevel(mod,L->car);
      L = L->cdr;
      if(L==NULL) break;
    }


    char *error = NULL;
    LLVMVerifyModule(mod, LLVMAbortProcessAction, &error);
    LLVMDisposeMessage(error);

/*    LLVMExecutionEngineRef engine;
    error = NULL;
    LLVMLinkInMCJIT();
    LLVMInitializeNativeTarget();
    LLVMInitializeNativeAsmPrinter();
    LLVMInitializeNativeAsmParser();
    if (LLVMCreateExecutionEngineForModule(&engine, mod, &error) != 0) {
        fprintf(stderr, "failed to create execution engine\n");
        abort();
    }
    if (error) {
        fprintf(stderr, "error: %s\n", error);
        LLVMDisposeMessage(error);
        exit(EXIT_FAILURE);
    }*/


    // Write out bitcode to file
    if (LLVMWriteBitcodeToFile(mod, output_filename) != 0) {
        fprintf(stderr, "error writing bitcode to file, skipping\n");
    }


    return 0;
}

