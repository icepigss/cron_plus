#include "ps_cron.h"

int
get_job_mem(const pid_t p)
{
    int i;
    char name[32];
    int vmrss;
    char file[64] = {0};
    FILE *fd;
    char line_buff[256] = {0};
    char *ret, *ret1;

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

void
acquire_daemonlock(int jobid)
{
    int fd;
    char pidfile[100], *h;
    char buf[3*PS_MAXLINE];
    ssize_t num;

	if (jobid) {
		snprintf(pidfile, sizeof(pidfile), "%s_%d", PS_JOB_PID, jobid);
	} else {
		snprintf(pidfile, sizeof(pidfile), "%s", PS_CRON_PID);
	}
    if ((fd = open(pidfile, O_RDWR|O_CREAT, 0600)) == -1) {
		sprintf(buf, "%s create or open file error: (%s)", ProgramName, strerror(errno));	
		ps_write_stderr(buf);
		ps_file_log(buf, 0);
        _exit(PS_FAILURE);
    }

    if (flock(fd, LOCK_EX|LOCK_NB) < 0) {
		FILE *fp;
		char *p;
		fp = fopen(pidfile ,"rb");
		fseek(fp, 0L, SEEK_END);
		int flen = ftell(fp);
		p = (char *)malloc(flen+1);
		fseek(fp, 0L, SEEK_SET);
		fread(p, 10, 1, fp);
		p[flen-1] = '\0';
		fclose(fp);
		sprintf(buf, "%s is already running: %s (%s)", ProgramName, p, strerror(errno));	
		ps_write_stderr(buf);
		ps_file_log(buf, 0);
        _exit(PS_FAILURE);
    }
    (void) fchmod(fd, 0644);
    (void) fcntl(fd, F_SETFD, 1);

    sprintf(buf, "%ld\n", (long)getpid());
    (void) lseek(fd, (off_t)0, SEEK_SET);
    num = write(fd, buf, strlen(buf));
    (void) ftruncate(fd, num);
}

char *
trim(char *str, char *terms)
{
    char *p = str;
    char *p1;
    if(p)
    {
        p1 = p + strlen(str) - 1;
        while(*p && strchr(terms, *p)) p++;
        while(p1 > p && isspace(*p1)) *p1-- = '\0';
    }
    return p;
}

int
get_char(FILE *f)
{
    int c;
    c = getc(f);
    if (c == '\n') {
        Set_LineNum(LineNumber + 1);
    }
    return c;
}

char *
get_string(char *s, FILE *f, char *terms)
{
    int c;
    while (EOF != (c = get_char(f)) && !strchr(terms, c)) {
        *s++ = (char) c;
    }

    *s = '\0';
    return s;

}

int
preg_match_cmd(char *cmd, char (*arr)[PS_MAXCMD])
{
    char match[PREG_MAX];
    regex_t reg;
    int err,nm = 10;
    int i = 0;
    int k = 0;
    regmatch_t pmatch[nm];

    if (regcomp(&reg, CMD_PATTERN, REG_EXTENDED) < 0) {
        return PS_ERROR;
    }

    err = regexec(&reg, cmd, nm, pmatch, 0);

    if (err == REG_NOMATCH) {
        return PS_ERROR;
    } 
    if (err) {
        return PS_ERROR;
    }

    for (i = 0; i < 10 && pmatch[i].rm_so != -1; i++) {
        int len = pmatch[i].rm_eo - pmatch[i].rm_so;
        if (len){
            memset(match,'\0', sizeof(match));
            memcpy(match,cmd+pmatch[i].rm_so,len);
            strcpy(arr[k++], match);
        }
    }

    return PS_OK;
}
