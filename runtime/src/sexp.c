#include <bearlang/common.h>
#include <bearlang/sexp.h>
#include <bearlang/mpc.h>

#include <stdio.h>

mpc_parser_t* Number;
mpc_parser_t* Symbol;
mpc_parser_t* Sexpr;
mpc_parser_t* Expr;
mpc_parser_t* Lispy;

void bl_init_parser() {
     Number = mpc_new("number");
     Symbol = mpc_new("symbol");
     Sexpr  = mpc_new("sexpr");
     Expr   = mpc_new("expr");
     Lispy  = mpc_new("lispy");

     mpca_lang(MPCA_LANG_DEFAULT,
      "                                          \
        number : /-?[0-9]+/ ;                    \
        symbol : '+' | '-' | '*' | '/' ;         \
        sexpr  : '(' <expr>* ')' ;               \
        expr   : <number> | <symbol> | <sexpr> ; \
        lispy  : /^/ <expr>* /$/ ;               \
      ",
      Number, Symbol, Sexpr, Expr, Lispy);
}

bl_ast_node_t* mpc_to_bl(mpc_ast_t* T) {
      bl_ast_node_t* retval = (bl_ast_node_t*)GC_MALLOC(sizeof(bl_ast_node_t));

      if(strstr(T->tag,"number")) {
         retval->node_type = BL_VAL_TYPE_NUMBER;
         retval->node_val.i_val = atoi(T->contents);
         return retval;
      }
      if(strstr(T->tag,"symbol")) {
         int content_len = strlen(T->contents);
         retval->node_type      = BL_VAL_TYPE_SYMBOL;
         retval->node_val.s_val = (char*)GC_MALLOC(content_len+1);
         snprintf(retval->node_val.s_val,content_len+1,"%s",T->contents);
         return retval;
      }

      if(strcmp(T->tag,">") == 0) {
         retval->node_type   = BL_VAL_TYPE_LIST;
         retval->children    = (bl_ast_node_t**)GC_MALLOC(sizeof(bl_ast_node_t*)*T->children_num);
      }

      if(strstr(T->tag, "sexpr")) {
         retval->node_type   = BL_VAL_TYPE_LIST;
         retval->children    = (bl_ast_node_t**)GC_MALLOC(sizeof(bl_ast_node_t*)*T->children_num);
      }
      
      int i=0;
      int c=0;
      for(i=0; i < T->children_num; i++) {
          if (strcmp(T->children[i]->contents, "(") == 0) { continue; }
          if (strcmp(T->children[i]->contents, ")") == 0) { continue; }
          if (strcmp(T->children[i]->tag,  "regex") == 0) { continue; }
          bl_ast_node_t* parsed = mpc_to_bl(T->children[i]);
          retval->children[c] = parsed;
          c++;
      }
      retval->child_count = c;
      return retval;
}

bl_ast_node_t* bl_parse_sexp(char* sexp) {
      mpc_result_t   r;
      bl_ast_node_t* retval = NULL;
      // TODO - come up with a proper error handling approach
      if(mpc_parse("input",sexp, Lispy, &r)) {
         mpc_ast_t* mpc_ast = r.output;
         retval = mpc_to_bl(mpc_ast->children[1]);
         mpc_ast_delete(r.output);
      } else {
         mpc_err_print(r.error);
         mpc_err_delete(r.error);
         
      }
      return retval;
}

char* bl_ser_sexp(bl_ast_node_t* ast) {
      char* retval = (char*)GC_MALLOC(sizeof(char)*5);
      snprintf(retval, 5, "None");
      return retval;
}
