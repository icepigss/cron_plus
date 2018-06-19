#include "ps_cron.h"

void
ps_write_stderr(char *text)
{
    write(STDERR_FILENO, text, strlen(text));
}

void
ps_write_stdout(char *text)
{
    write(STDOUT_FILENO, text, strlen(text));
}

void
ps_file_log(char *text, int jobid)
{
    int fd;
    char msg[1000];
    char logfile[50];
	time_t rawtime;
	struct tm *ptminfo;

	time(&rawtime);
	ptminfo = localtime(&rawtime);

	if (jobid) {
		snprintf(logfile, sizeof(logfile), "%s_%d", PS_JOB_FILE, jobid);
	} else  {
		snprintf(logfile, sizeof(logfile), "%s", PS_LOG_FILE);
	}
    if ((fd = open(logfile, O_WRONLY|O_APPEND|O_CREAT, 0600)) == -1) {
		fprintf(stderr, "%s create or open file error (%s)\n", ProgramName, strerror(errno));	
        _exit(PS_FAILURE);
    }

	(void) fcntl(fd, F_SETFD, 1);

	snprintf(msg, sizeof(msg), "(%02d-%02d-%02d %02d:%02d:%02d) %s\n",
			ptminfo->tm_year + 1900, ptminfo->tm_mon + 1, ptminfo->tm_mday,
			ptminfo->tm_hour, ptminfo->tm_min, ptminfo->tm_sec, text);
	if (fd < 0 || write(fd, msg, strlen(msg)) < 0) {
		fprintf(stderr, "%s: can't write to log file (%s)\n", ProgramName, strerror(errno));
	}

	close(fd);
}
