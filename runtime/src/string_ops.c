#include <stdlib.h>
#include <stdio.h>

#include <bearlang/types.h>
#include <bearlang/utils.h>
#include <bearlang/list_ops.h>
#include <bearlang/string_ops.h>
#include <bearlang/sexp.h>

char* safe_strcat(char* a, char* b) { // LCOV_EXCL_LINE
      size_t a_len   = strlen(a);
      size_t b_len   = strlen(b);
      size_t new_len = a_len+b_len+1;
      char* retval = GC_MALLOC_ATOMIC(new_len);
      snprintf(retval,new_len,"%s%s",a,b);
      return retval;
}

char* tail_substr(char* s, char* sep) {
      char* ret = strstr(s, sep);
      if(ret==NULL) return NULL;
      *ret = '\0';
      return ret+strlen(sep);
}

bl_val_t* split_str(char* s, char* sep) {
	size_t s_len = strlen(s);
	size_t sep_len = strlen(sep);
	if(s_len == 0) return bl_mk_null();
	if(s_len <= sep_len) return bl_mk_list(1,bl_mk_str(s));

	size_t buf_len = s_len + sep_len + 1;
	char* buf  = GC_MALLOC_ATOMIC(buf_len);
	snprintf(buf,buf_len,"%s",s);

	bl_val_t* retval = NULL;
	char* tail = tail_substr(buf,sep);
	while(tail != NULL) {
		retval = bl_list_prepend(retval,bl_mk_str(buf));
		buf = tail;
		tail = tail_substr(buf,sep);
	}
	if(strlen(buf)>0) retval = bl_list_prepend(retval,bl_mk_str(buf));
	retval = bl_list_reverse(retval);
	return retval;
}
