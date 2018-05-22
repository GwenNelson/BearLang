#include <bearlang/common.h>
#include <bearlang/types.h>
#include <bearlang/ctx.h>
#include <bearlang/utils.h>
#include <bearlang/list_ops.h>
#include <bearlang/sexp.h>

#include "mongoose.h"

typedef struct simpleweb_html_resp_t {
    size_t content_len;
    char*  html_content;
} simpleweb_html_resp_t;

bl_val_t* bl_html_response(bl_val_t* ctx, bl_val_t* params) {
        params = bl_ctx_eval(ctx,params);
	bl_val_t* first = bl_list_first(params);
	simpleweb_html_resp_t* resp_obj = GC_MALLOC(sizeof(simpleweb_html_resp_t));
	resp_obj->html_content = bl_ser_naked_sexp(first);
	resp_obj->content_len  = strlen(resp_obj->html_content);
	return bl_mk_ptr(resp_obj);
}

static void ev_handler(struct mg_connection *c, int ev, void *p) {
  if (ev == MG_EV_HTTP_REQUEST) {
    struct http_message *hm = (struct http_message *) p;

    // get the details of the request
    char* req_method = GC_MALLOC_ATOMIC(hm->method.len+2);
    snprintf(req_method,hm->method.len+1,"%s", hm->method.p);
    char* uri        = GC_MALLOC_ATOMIC(hm->uri.len+2);
    snprintf(uri,hm->uri.len+1,"%s", hm->uri.p);
    char* body       = GC_MALLOC_ATOMIC(hm->body.len+2);
    snprintf(body,hm->body.len+1,"%s", hm->body.p);

    // get the handler function and ctx
    bl_val_t* handler_func = (bl_val_t*)c->mgr->user_data;
    bl_val_t* ctx = (bl_val_t*)handler_func->custom_data;

    // build BearLang expression
    bl_val_t* L = bl_mk_list(4,(bl_val_t*)c->mgr->user_data,
    		               bl_mk_str(req_method),
		               bl_mk_str(uri),
			       bl_mk_str(body));

    // eval the expression
    bl_val_t* resp = bl_ctx_eval(ctx,L);

    // pull out the resp_obj
    simpleweb_html_resp_t* resp_obj = (simpleweb_html_resp_t*)resp->c_ptr;


    // We have received an HTTP request. Parsed request is contained in `hm`.
    // Send HTTP reply to the client which shows full original request.
    mg_send_head(c, 200, (int64_t)resp_obj->content_len, "Content-Type: text/html");
    mg_printf(c, resp_obj->html_content);
  }
}

bl_val_t* bl_serve(bl_val_t* ctx, bl_val_t* params) {
	params = bl_ctx_eval(ctx,params);
	bl_val_t* portnum      = bl_list_first(params);
	bl_val_t* handler_func = bl_list_second(params);
        handler_func->custom_data = ctx;

        // shamelessly ripped from mongoose example code

	struct mg_mgr mgr;
        struct mg_connection *c;

        mg_mgr_init(&mgr, (void*)handler_func);
        c = mg_bind(&mgr, bl_ser_sexp(portnum), ev_handler);
        mg_set_protocol_http_websocket(c);

        for (;;) {
           mg_mgr_poll(&mgr, 1000);
        }
        mg_mgr_free(&mgr);
}

bl_val_t* bl_mod_init(bl_val_t* ctx) {
     bl_val_t* my_ctx = bl_ctx_new(ctx);
     bl_ctx_set(my_ctx, bl_mk_symbol("html_response"), bl_mk_native_oper(&bl_html_response));
     bl_ctx_set(my_ctx, bl_mk_symbol("serve"),         bl_mk_native_oper(&bl_serve));
     return my_ctx;
}
