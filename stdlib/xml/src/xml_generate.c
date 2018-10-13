// Used for implementing SXML-type XML generation

#include <bearlang/common.h>
#include <bearlang/types.h>
#include <bearlang/ctx.h>
#include <bearlang/utils.h>
#include <bearlang/list_ops.h>
#include <bearlang/string_ops.h>
#include <bearlang/sexp.h>

static char* bl_module_name        = "xml";
static char* bl_module_summary     = "tools for generating and parsing XML";
static char* bl_module_description = "This module provides tools for converting between s-expressions and XML using SXML style expressions";
static char* bl_module_example     = ""
"(import xml)\n"
"(using xml::x)\n"
"(x::html \n"
"     (x::head (x::title \"An example\"))\n"
"     (x::body\n"
"         (x::h1 (@ 'id \"greeting\") \"Hi there!\")\n"
"         (x::p \"This is an example.\"))\n"
"\n"
" <html>\n"
"    <head><title>An example</title></head>\n"
"    <body>\n"
"       <h1 id=\"greeting\">Hi there!</h1>\n"
"       <p>This is an example.</p>\n"
"    </body>\n"
" </html>\n";


bl_val_t* xml_gen_oper(bl_val_t* ctx, bl_val_t* params) {
     bl_val_t* sym_name = params->invoked_sym;
     char* tag_name_s = strrchr(sym_name->s_val,':')+1;
     bl_val_t* tag_name = bl_mk_str(tag_name_s);
     bl_val_t* ret_list  = NULL;
     bl_val_t* attr_list = NULL;
     bl_val_t* i = NULL;
     bl_val_t* L = params;
     if(bl_list_len(L)==0) {
        return bl_mk_str(safe_strcat(safe_strcat("<", tag_name_s),"/>"));
     }
     for(i=L; i != NULL; i=i->cdr) {
         if(i->car->type == BL_VAL_TYPE_CONS) {
            if(i->car->car->type == BL_VAL_TYPE_SYMBOL) {
               if(strcmp(i->car->car->s_val,"@")==0) {
                  attr_list = bl_list_append(attr_list,bl_eval(ctx,i->car->cdr));
               } else {
                  ret_list  = bl_list_append(ret_list,bl_mk_str(bl_ser_naked_sexp(bl_eval(ctx,i->car))));
               }
            } else {
               ret_list  = bl_list_append(ret_list,bl_mk_str(bl_ser_naked_sexp(bl_eval(ctx,i->car))));
            }
         } else {
           ret_list  = bl_list_append(ret_list,bl_mk_str(bl_ser_naked_sexp(bl_eval(ctx,i->car))));
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

bl_val_t* xml_escape(bl_val_t* ctx, bl_val_t* params) {
     params = bl_eval(ctx,params);
     bl_val_t* s = bl_list_first(params);
     char* escaped = s->s_val;
     escaped = str_replace(escaped,"<","&lt;");
     escaped = str_replace(escaped,">","&gt;");
     return bl_mk_str(escaped);
}

bl_val_t* bl_mod_init(bl_val_t* ctx) {
     bl_val_t* my_ctx  = bl_ctx_new(ctx);
     bl_val_t* xml_ctx = bl_ctx_new(ctx);
     xml_ctx->ctx_get      = &x_get;
     bl_ctx_set(my_ctx,bl_mk_symbol("x"),xml_ctx);
     bl_ctx_set(my_ctx,bl_mk_symbol("escape"), bl_mk_native_oper(&xml_escape));

     bl_ctx_set(my_ctx,bl_mk_symbol("*NAME*"),       bl_mk_str(bl_module_name));
     bl_ctx_set(my_ctx,bl_mk_symbol("*SUMMARY*"),    bl_mk_str(bl_module_summary));
     bl_ctx_set(my_ctx,bl_mk_symbol("*DESCRIPTION*"),bl_mk_str(bl_module_description));
     bl_ctx_set(my_ctx,bl_mk_symbol("*EXAMPLE*"),    bl_mk_str(bl_module_example));
     return my_ctx;
}
