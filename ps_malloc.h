#ifndef _H_PS_MALLOC_
#define _H_PS_MALLOC_

void *ps_malloc(size_t size);
void ps_free(void *ptr);
void *pv_calloc(size_t nmemb, size_t size);
void *ps_realloc(void *ptr, size_t size);
char *ps_copy_str(const char *src);

#endif
