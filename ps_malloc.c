#include "ps_cron.h"

void *
ps_malloc(size_t size)
{
    void *ptr = malloc(size);
    assert(ptr != NULL);
    return ptr;
}

void 
ps_free(void *ptr)
{
    free(ptr);
}

void *
ps_calloc(size_t nmemb, size_t size)
{
    void *ptr = calloc(nmemb, size);
    assert(ptr != NULL);
    return ptr;
}

/*
void *
ps_realloc(void *ptr, size_t size)
{
    ptr = realloc(ptr, size);
    assert(ptr != NULL);
    return ptr;
}

char *
ps_copy_str(const char *src)
{
    char *dest;
    dest = strdup(src);
    assert(dest != NULL);
    return dest;
}
*/
