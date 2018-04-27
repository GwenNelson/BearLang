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

LLVMTypeRef bl_val_t_cons;

LLVMModuleRef create_module(const char* mod_name) {
   LLVMModuleRef retval = LLVMModuleCreateWithName(mod_name);
   
   return retval;
}

bl_val_t* get_code(char* filename) {
   fprintf(stderr,"Reading from %s\n", filename);
   FILE* fd = fopen(filename,"r");
   bl_val_t* retval = bl_parse_file(filename,fd);
   fclose(fd);
   return retval;
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


     LLVMBuildRet(builder, tmp);
}

int main(int argc, char** argv) {
    GC_INIT();
    bl_init();

    char* filename = argv[1];

    bl_val_t* code = get_code(filename);
    LLVMModuleRef mod = create_module(filename);
    bl_val_t* L = code;

    while(L->car != NULL) {
      handle_toplevel(mod,L->car);
      L = L->cdr;
      if(L==NULL) break;
    }


    char *error = NULL;
    LLVMVerifyModule(mod, LLVMAbortProcessAction, &error);
    LLVMDisposeMessage(error);

    LLVMExecutionEngineRef engine;
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
    }


    // Write out bitcode to file
    if (LLVMWriteBitcodeToFile(mod, "output.bc") != 0) {
        fprintf(stderr, "error writing bitcode to file, skipping\n");
    }

//    LLVMDisposeBuilder(builder);
    LLVMDisposeExecutionEngine(engine);

    return 0;
}

