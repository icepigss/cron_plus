#ifndef _H_PS_GLOBALS_
#define _H_PS_GLOBALS_

#include "ps_macros.h"

#define PS_VERSION  "1.0"

#define PS_LINEFEED	"\x0a"

#define PS_RESULT   int
#define PS_TRUE     1 
#define PS_FALSE    0 

#define PS_OK       1 
#define PS_ERROR    0 

#define PS_SUCCESS  1 
#define PS_FAILURE  0 

#define PS_CONFIG_MYSQL  0
#define PS_CONFIG_FILE   1 

#define PS_VMRSS_LINE 16 

#define PS_MAX_JOB 1000

#define PS_MAXLINE 1024 

#define PS_MAXCMD 1024 

#define PREG_MAX 1000

#define TIME_MAP_LEN 64

#define SECONDS_PER_MINUTE 60

#define MINUTES_PER_HOUR 60

#define HOURS_PER_DAY 24

#define DAYS_PER_WEEK 7

#define MONS_PER_YEAR 12

#define MINUTES_PER_DAY MINUTES_PER_HOUR*HOURS_PER_DAY
#define MINUTES_PER_WEEK MINUTES_PER_DAY*DAYS_PER_WEEK

#define PS_RET_OK "OK"

#define PS_LINEFEED "\x0a"

#define PS_CRON_FILE   "cron.conf" 

#define WORK_DIR "pcron" 

#define VAR_DIR "pcron/var" 

#define ETC_DIR "pcron/etc" 

#define LOG_DIR "pcron/log" 

#define JOB_DIR "pcron/jobs" 

#define PS_CRON_PID "var/prcon.pid" 

#define PS_JOB_PID "var/job" 

#define PS_LOG_FILE "log/pcron.log" 

#define PS_DEBUG_FILE "log/debug.log" 

#define PS_JOB_FILE "jobs/job" 

#define _PATH_DEVNULL "/dev/null"

#define STDIN	0

#define STDOUT	1

#define STDERR	2

#define TIME_TYPE_MIN  1

#define TIME_TYPE_HOUR 2

#define TIME_TYPE_DAY  3 

#define TIME_TYPE_WEEK 4

#define TIME_TYPE_MON 5 

#define TIME_FORM_BAD (unsigned long)1 << 61

//#define CMD_PATTERN  "(\\*|[0-9]|[1-5][0-9]|\\*/[1-9]+) (\\*|[0-9]|[1-5][0-9]|\\*/[1-9]+) (\\*|[0-9]|[1-5][0-9]|\\*/[1-9]+) (\\*|[0-9]|[1-5][0-9]|\\*/[1-9]+) (\\*|[0-9]|[1-5][0-9]|\\*/[1-9]+) (.*)"
#define CMD_PATTERN  "([0-9\\*,-/]*) ([0-9\\*,-/]*) ([0-9\\*,-/]*) ([0-9\\*,-/]*) ([0-9\\*,-/]*) (.*)"

char   *ProgramName;

int    LineNumber;

static int MaxJobId = 0;

int    DebugFlags;

time_t StartTime;

int    ConfigMode;


#endif
