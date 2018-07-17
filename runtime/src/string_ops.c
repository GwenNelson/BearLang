#include <stdlib.h>
#include <stdio.h>

#include <bearlang/types.h>
#include <bearlang/utils.h>
#include <bearlang/list_ops.h>
#include <bearlang/string_ops.h>
#include <bearlang/sexp.h>

char* safe_strcat(char* a, char* b) { // LCOV_EXCL_LINE
      if(a==NULL) return b;
      if(b==NULL) return a;
      size_t a_len   = strlen(a);
      size_t b_len   = strlen(b);
      size_t new_len = a_len+b_len+1;
      char* retval = GC_MALLOC_ATOMIC(new_len);
      snprintf(retval,new_len,"%s%s",a,b);
      return retval;
}

char* join_str(bl_val_t* L, char* sep) { // LCOV_EXCL_LINE
      char* retval = NULL;
      bl_val_t* i=L;
      for(i=L; i != NULL; i=i->cdr) {
          if(retval!=NULL) retval = safe_strcat(retval,sep);
	  retval = safe_strcat(retval,bl_ser_naked_sexp(i->car));
      }
      return retval;
}

char* tail_substr(char* s, char* sep) { // LCOV_EXCL_LINE
      char* ret = strstr(s, sep);
      if(ret==NULL) return NULL;
      *ret = '\0';
      return ret+strlen(sep);
}

bl_val_t* split_str(char* s, char* sep) { // LCOV_EXCL_LINE
	size_t s_len = strlen(s);
	size_t sep_len = strlen(sep);
	if(s_len == 0) return bl_mk_null(); // LCOV_EXCL_LINE
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

char* str_replace(char* s, char* a, char* b) {
      char* retval = NULL;
      if(strstr(s,a)==NULL) { // if it's not there, we just return s
         return s;
      }
      bl_val_t* L = split_str(s,a);
      retval = join_str(L,b);
      return retval;
}
