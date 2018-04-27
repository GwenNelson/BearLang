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

FILE* output_fd=NULL;

/*LLVMModuleRef create_module(const char* mod_name) {
   LLVMModuleRef retval = LLVMModuleCreateWithName(mod_name);
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
     LLVMDisposeBuilder(builder);
}*/

typedef enum c_types_t {
     C_INT,
     C_2D_CHAR_ARRAY,
} c_types_t;

const char* c_type_to_str(c_types_t T) {
      switch(T) {
         case C_INT:
              return "int";
	 break;
	 case C_2D_CHAR_ARRAY:
	      return "char**";
	 break;
	 default:
	      return "";
	 break;
      }
}

void generate_c_func(char* func_name, c_types_t* arg_types, char** arg_names, c_types_t ret_type) {
     while(arg_names) {

     }
}

void handle_toplevel(bl_val_t* E) {
     bl_val_t* func_name = bl_list_second(E);
     bl_val_t* func_args = bl_list_third(E);
     bl_val_t* func_body = bl_list_rest(bl_list_rest(bl_list_rest(E)));
     printf("Generating function %s with args %s and %llu expressions\n", bl_ser_sexp(func_name),
		                                                          bl_ser_sexp(func_args),
							                  bl_list_len(func_body));

}


bl_val_t* get_code(char* filename) {
   fprintf(stderr,"Reading from %s\n", filename);
   FILE* fd = fopen(filename,"r");
   bl_val_t* retval = bl_parse_file(filename,fd);
   fclose(fd);
   return retval;
}

int main(int argc, char** argv) {
    GC_INIT();
    bl_init();

    char* filename = argv[1];

    bl_val_t* code = get_code(filename);
//    LLVMModuleRef mod = create_module(filename);
    bl_val_t* L = code;

    char output_filename[1024];
    snprintf(output_filename,1024,"%s.c", filename);

    output_fd = fopen(output_filename,"w");

    while(L->car != NULL) {
      handle_toplevel(L->car);
      L = L->cdr;
      if(L==NULL) break;
    }


/*    char *error = NULL;
    LLVMVerifyModule(mod, LLVMAbortProcessAction, &error);
    LLVMDisposeMessage(error);*/

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
/*    if (LLVMWriteBitcodeToFile(mod, "output.bc") != 0) {
        fprintf(stderr, "error writing bitcode to file, skipping\n");
    }*/


    return 0;
}

