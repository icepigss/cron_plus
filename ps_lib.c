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
