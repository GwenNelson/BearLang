// Used for implementing SXML-type XML generation
// (import xml)
// (using xml::x)
// (x::html 
//     (x::head (x::title "An example"))
//     (x::body
//         (x::h1 (@ 'id "greeting") "Hi there!")
//         (x::p "This is an example."))
//
// <html>
//    <head><title>An example</title></head>
//    <body>
//       <h1 id="greeting">Hi there!</h1>
//       <p>This is an example.</p>
//    </body>
// </html>

#include <bearlang/common.h>
#include <bearlang/types.h>
#include <bearlang/ctx.h>
#include <bearlang/utils.h>
#include <bearlang/list_ops.h>
#include <bearlang/sexp.h>

bl_val_t* xml_gen_oper(bl_val_t* ctx, bl_val_t* params) {
     bl_val_t* sym_name = params->invoked_sym;
     char* tag_name_s = strrchr(sym_name->s_val,':')+1;
     bl_val_t* tag_name = bl_mk_str(tag_name_s);
     bl_val_t* ret_list  = NULL;
     bl_val_t* attr_list = NULL;
     bl_val_t* i = NULL;
     bl_val_t* L = params;
     for(i=L; i != NULL; i=i->cdr) {
         if(i->car->type == BL_VAL_TYPE_CONS) {
            if(i->car->car->type == BL_VAL_TYPE_SYMBOL) {
               if(strcmp(i->car->car->s_val,"@")==0) {
                  attr_list = bl_list_append(attr_list,bl_ctx_eval(ctx,i->car->cdr));
               } else {
                  ret_list  = bl_list_append(ret_list,bl_mk_str(bl_ser_naked_sexp(bl_ctx_eval(ctx,i->car))));
               }
            } else {
               ret_list  = bl_list_append(ret_list,bl_mk_str(bl_ser_naked_sexp(bl_ctx_eval(ctx,i->car))));
            }
         } else {
           ret_list  = bl_list_append(ret_list,bl_mk_str(bl_ser_naked_sexp(bl_ctx_eval(ctx,i->car))));
         }
     }

     char* retval = GC_MALLOC_ATOMIC(strlen(tag_name->s_val)+8);
     if(attr_list==NULL) { // no attributes in the tag, empty list
        if(ret_list==NULL) { // no contents inside the tag
           sprintf(retval,"<%s/>", tag_name->s_val);
        } else {
           sprintf(retval,"<%s>", tag_name->s_val);
        }
     } else {
        i = NULL;
        L = attr_list;
        sprintf(retval,"<%s", tag_name->s_val);
        for(i=L; i != NULL; i=i->cdr) {
            retval = GC_REALLOC(retval,strlen(retval)+8+strlen(i->car->car->s_val)+strlen(i->car->cdr->car->s_val));
            sprintf(retval,"%s %s=\"%s\"",retval,i->car->car->s_val,i->car->cdr->car->s_val);
        }
        retval = GC_REALLOC(retval,strlen(retval)+4);
        sprintf(retval,"%s>", retval);
     }
     if(ret_list != NULL) {
        i = NULL;
        L = ret_list;
        for(i=L; i != NULL; i=i->cdr) {
            retval = GC_REALLOC(retval,strlen(retval)+8+strlen(i->car->s_val));
            sprintf(retval,"%s%s",retval,i->car->s_val);
        }
        retval = GC_REALLOC(retval,strlen(retval)+strlen(tag_name->s_val)+8);
        sprintf(retval,"%s</%s>", retval,tag_name->s_val);
     }

     return bl_mk_str(retval);
}

bl_val_t* x_get(bl_val_t* ctx, bl_val_t* sym) {
     bl_val_t* retval = bl_mk_native_oper(&xml_gen_oper);
     return retval;
}

bl_val_t* bl_mod_init(bl_val_t* ctx) {
     bl_val_t* my_ctx  = bl_ctx_new(ctx);
     bl_val_t* xml_ctx = bl_ctx_new(ctx);
     xml_ctx->ctx_get      = &x_get;
     bl_ctx_set(my_ctx,bl_mk_symbol("x"),xml_ctx);
     return my_ctx;
}
