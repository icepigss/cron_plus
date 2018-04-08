#include <stdlib.h>
#include <unistd.h> 
#include <assert.h>

ps_int_t
get_job_mem(const pid_t p)
{
    int i;
    char name[32];
    int vmrss;
    char file[64] = {0};
    FILE *fd;
    char line_buff[256] = {0};
    char *ret, ret1;

    sprintf(file,"/proc/%d/status",p);
    fd = fopen (file, "r");

    for (i = 0; i < PS_VMRSS_LINE-1; i++)
    {
        ret = fgets(line_buff, sizeof(line_buff), fd);
    }
    ret1  = fgets(line_buff, sizeof(line_buff), fd);
    sscanf(line_buff, "%s %d", name,&vmrss);

    fclose(fd);

    return vmrss;
}

int
glue_strings(char *buffer, size_t buffer_size, const char *a, const char *b,
        char separator)
{
    char *buf;
    char *buf_end;

    if (buffer_size <= 0)
        return (0);
    buf_end = buffer + buffer_size;
    buf = buffer;

    for ( /* nothing */; buf < buf_end && *a != '\0'; buf++, a++ )
        *buf = *a;
    if (buf == buf_end)
        return (0);
    if (separator != '/' || buf == buffer || buf[-1] != '/')
        *buf++ = separator;
    if (buf == buf_end)
        return (0);
    for ( /* nothing */; buf < buf_end && *b != '\0'; buf++, b++ )
        *buf = *b;
    if (buf == buf_end)
        return (0);
    *buf = '\0';
    return (1);
}
