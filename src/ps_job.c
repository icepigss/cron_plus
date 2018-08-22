#include "ps_cron.h"

static int days_per_mon[MONS_PER_YEAR+1] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}; 

void
exit_job_ps()
{
    _exit(PS_SUCCESS);
}

void
job_callback(joblist_t *job, int signo)
{
	// SIGKILL SIGSEGV SIGALRM
	char str[100];
	sprintf(str, "job %d end : %d", job->jobid, signo);
	ps_file_log(str, 0);
	ps_file_log(str, job->jobid);
    if (DebugFlags == 1) {
        printf("working ending:%d\n", signo);
    }
    // TODO callback
}

void
child_process(joblist_t *job)
{
    pid_t pid;

    pid = vfork();
    switch (pid) {
        case -1:
            _exit(PS_FAILURE);
            break;
        case 0:
			{
			int flag = 0;
			int i = 0;
			char shell[50];
			char arg1[50];
			char *p;

			(void) setsid();
			dup2(STDOUT, STDERR);
			alarm(job->time_out);

			memset(shell, '\0', sizeof(shell));
			memset(arg1, '\0', sizeof(arg1));

			p = job->command;

			while (*p != '\0' && *p != '\t' && *p != ' ') {
				shell[i++] = *p;
				p++;
			}
			shell[i] = '\0';
			i = 0;
			if (*p != '\0') {
				while (*p == '\t' || *p == ' ') {
					p++;
				}
				if (*p != '\0') {
					while (*p != '\0') {
						arg1[i++] = *p;
						p++;
					}
					arg1[i] = '\0';
				}
			}

			if (DebugFlags == 1) {
				printf("cmd:%s\n", job->command);
				printf("shell:%s\n", shell);
				printf("arg1:%s\n", arg1);
			}
			if(-1 == execl(shell, shell, arg1, NULL)) {
				perror("execl error\n");
				_exit(PS_FAILURE);
			}
			}
            break;
        default:
			{
				int pr;
				int status;
				int mem_usage;
				int sig = 0;
				do { 
					pr = waitpid(pid, &status, WNOHANG);
					if (pr == 0) {
						/*
						mem_usage = get_job_mem(pid);
						printf("mmmmmmmm:%d\n", mem_usage);
						if (mem_usage > job->mem_limit) {
							kill(pid, SIGKILL);
						}
						*/
						sleep(1);
					}
				} while (pr == 0);

				if(!WIFEXITED(status)) {
					sig = WTERMSIG(status);
					if(WIFSIGNALED(status)) {
						sig = WTERMSIG(status);
					} else {
						;
					}
				}
				job_callback(job, sig);
			}
			break;
    }
}

void
do_command(joblist_t *job)
{
	char str[100];

    switch (fork()) {
        case -1:
            _exit(PS_FAILURE);
            break;
        case 0:
			acquire_daemonlock(job->jobid);
			sprintf(str, "job %d start", job->jobid);
			ps_file_log(str, 0);
			ps_file_log(str, job->jobid);
            child_process(job);
            _exit(PS_SUCCESS);
            break;
        default:
            break;
    }
}

void 
refresh_job(joblist_t *job)
{
}

void
get_unit_from_timstamp(time_t time, int *min, int *hour, int *day,
        int *week, int *mon, int *year)
{
    struct tm *timenow;
    timenow = localtime(&time);

    *min = timenow->tm_min;
    *hour = timenow->tm_hour;
    *day = timenow->tm_mday;
    *week = timenow->tm_wday;
    *mon = timenow->tm_mon+1;
    *year = timenow->tm_year;
}

unsigned long
gettime_by_format(time_st *min, time_st *hour, 
		time_st *day, time_st *week, time_st *mon, unsigned long last_execute_min)
{
	int c_min, c_hour, c_day, c_week, c_mon, c_year;
    int min_diff, hour_diff, day_diff, week_diff, mon_diff;
    int hour_index, day_index, mon_index;
	time_t now;
	struct tm *timenow;
	unsigned long current_min, next_min;
	int GMToff;

	time(&now);
	timenow = localtime(&now);
	GMToff = get_gmtoff(&now, timenow);
    
	current_min = Time_To_Min(now, GMToff);
    next_min = current_min + 1;

    get_unit_from_timstamp(now, &c_min, &c_hour, &c_day, &c_week, &c_mon, &c_year);

    if (DebugFlags == 1) {
        printf("current_min:%d\n", c_min);
        printf("current_hour:%d\n", c_hour);
        printf("current_day:%d\n", c_day);
        printf("current_week:%d\n", c_week);
        printf("current_mon:%d\n", c_mon);
        printf("current_year:%d\n", c_year);
        printf("c_mins:%lu\n", current_min);
        printf("l_mins:%lu\n", last_execute_min);
    }

    hour_index = get_current_index(hour->value, c_hour);
    day_index = get_current_index(day->value, c_day);
    mon_index = get_current_index(mon->value, c_mon);

    if ((hour_index == 0 || day_index == 0 || mon_index == 0) && last_execute_min == 0) {
        min_diff = get_time_diff_first(min->value, c_min);
    } else {
        min_diff = get_time_diff_next(min->value, c_min, 0);
    }
    if ((day_index == 0 || mon_index == 0) && last_execute_min == 0) {
        hour_diff = get_time_diff_first(hour->value, c_hour);
    } else {
        hour_diff = get_time_diff_next(hour->value, c_hour, min_diff);
    }
    if (min_diff <= 0) {
        hour_diff--;
        min_diff += MINUTES_PER_HOUR + hour_diff*MINUTES_PER_HOUR;
    } else {
        min_diff += hour_diff*MINUTES_PER_HOUR;
    }
    if (mon_index == 0 && last_execute_min == 0) {
        day_diff = get_time_diff_first(day->value, c_day);
    } else {
        day_diff = get_time_diff_next(day->value, c_day, min_diff);
    }
    if (min_diff <= 0) {
        day_diff--;
        min_diff += MINUTES_PER_HOUR*HOURS_PER_DAY + MINUTES_PER_HOUR*HOURS_PER_DAY*day_diff;
    } else {
        min_diff += MINUTES_PER_HOUR*HOURS_PER_DAY*day_diff;
    }
    mon_diff = get_time_diff_next(mon->value, c_mon, min_diff);
    if (min_diff <= 0) {
        mon_diff--;
        min_diff += MINUTES_PER_HOUR*HOURS_PER_DAY*get_days_of_mon(c_year, c_mon, 1) \
                    + MINUTES_PER_HOUR*HOURS_PER_DAY*get_days_of_mon(c_year, c_mon, mon_diff);
    } else {
        min_diff += MINUTES_PER_HOUR*HOURS_PER_DAY*get_days_of_mon(c_year, c_mon, mon_diff);
    }
    return current_min + min_diff;
}

int
get_days_of_mon(int year, int mon, int diff)
{
    int ret = 0;
    int i = mon;
    int end = mon+diff;
    if (diff == 0) {
        return 0;
    }
    if (end > mon) {
        while (i < end) {
            if (((!(year % 100) && !(year % 400)) || ((year % 100) && !(year % 4))) && i == 2) {
                ret += 28;
            } else {
                ret += days_per_mon[i];
            }
            i++;
        }
    } else {
        while (i <= 12) {
            if (((!(year % 100) && !(year % 400)) || ((year % 100) && !(year % 4))) && i == 2) {
                ret += 28;
            } else {
                ret += days_per_mon[i];
            }
            i++;
        }
        i = 1;
        year++;
        while (i < end) {
            if (((!(year % 100) && !(year % 400)) || ((year % 100) && !(year % 4))) && i == 2) {
                ret += 28;
            } else {
                ret += days_per_mon[i];
            }
            i++;
        }
    }

    return ret;
}

/**
 * check if current time is executable
 */
int
get_current_index(unsigned long value, int c_time)
{
    for (int i = 0; i < TIME_MAP_LEN-3; i++ ) {
        if ((value >> i) & (unsigned long)1) {
            if (c_time == i) {
                return 1;
            }
        }
    }
    return 0;
}

int
get_time_diff_first(unsigned long value, int c_time)
{
    int least = 100;
    for (int i = 0; i < TIME_MAP_LEN-3; i++ ) {
        if ((value >> i) & (unsigned long)1) {
            least = __min(least, i);
        }
    }
    // if '*', diff will be set to zero
    if (least == 100) {
        return 0;
    }

    return least - c_time;
}

int
get_time_diff_next(unsigned long value, int c_time, int flag)
{
    int least = 100;
    int diff = 100;
    for (int i = 0; i < TIME_MAP_LEN-3; i++ ) {
        if ((value >> i) & (unsigned long)1) {
            least = __min(least, i);
            if ((flag > 0 && i == c_time) || i > c_time) {
                diff = i - c_time;
                break;
            }
        }
    }
    // if '*', diff will be set to zero
    if (least == 100) {
        return 0;
    }
    if (diff == 100) { 
        diff = least - c_time;
    }

    return diff;
}

void 
add_job(config_t *config, joblist_t *job, unsigned long last_execute_time)
{
	unsigned long timeframe;
	cronqueue_t *p, *pre, *n, *cron;
	cron = config->cron;
	/*
	 * init cron_list
	 */
	if (NULL == cron) {
		cron = (cronqueue_t*)malloc(sizeof(cronqueue_t));
		cron->list = NULL;
		cron->next = NULL;
		cron->timeframe = 0;
		config->cron = cron;
	}
	
	timeframe = gettime_by_format(&job->min, &job->hour, &job->day,
			&job->week, &job->mon, last_execute_time);	

    char s[100];
    char c[100];
    sprintf(s, "jobid:%d\tlast_execute_time:%lu", job->jobid, last_execute_time);
    sprintf(c, "jobid:%d\ttimeframe:%lu", job->jobid, timeframe);
    ps_file_log(s, 0);
    ps_file_log(c, 0);

	if (NULL == cron->list) {
		cron->list = job;
		job->next = NULL;
		cron->timeframe = timeframe;
		cron->next = NULL;
	} else {
		pre = cron;
		p = pre->next;
		if (timeframe < pre->timeframe) {
			n = (cronqueue_t*)malloc(sizeof(cronqueue_t));
			n->next = pre;
			config->cron = n;
			n->timeframe = timeframe;
			n->list = job;
			job->next = NULL;
		} else if (timeframe == pre->timeframe) {
			job->next = pre->list;
			pre->list = job;
		} else {
			int flag = 0;
			while (p != NULL) {
				if (timeframe == p->timeframe) {
					job->next = p->list;
					p->list = job;
					flag = 1;
					break;
				} else if (timeframe < p->timeframe) {
					n = (cronqueue_t*)malloc(sizeof(cronqueue_t));
					n->next = p;
					pre->next = n;
					n->timeframe = timeframe;
					n->list = job;
					job->next = NULL;
					flag = 1;
					break;
				}
				pre = p;
				p = pre->next;
			}
			if (flag == 0) {
				n = (cronqueue_t*)malloc(sizeof(cronqueue_t));
				n->next = NULL;
				pre->next = n;
				n->timeframe = timeframe;
				n->list = job;
				job->next = NULL;
			}
		}
	}
}

void
find_job(joblist_t *job, cronqueue_t *cron_queue, int jobid)
{
	joblist_t * j;
	//return j;
}

void 
delete_job(cronqueue_t *cron_queue, joblist_t *job)
{
}

/**
 * perform tasks from cron-list,
 * just check if the fist job-list meets the condition that job-time equal current time
 * redistribute jobs to the cron-list after execute them
 */
void 
run_job(config_t *config)
{
    cronqueue_t *first = NULL;
    joblist_t *dj, *aj, *p = NULL;
	unsigned long execute_time;
	time_t now;
	struct tm *timenow;
	int GMToff;

	time(&now);
	timenow = localtime(&now);
	GMToff = get_gmtoff(&now, timenow);

	first = config->cron;
	execute_time = first->timeframe;

    if (first->list == NULL) {
        return;
    }
    printf("sunsongsssnn:%ld\n", (now + GMToff) / (time_t)SECONDS_PER_MINUTE);
    printf("sunsongsssll:%ld\n", execute_time);
	if (Time_To_Min(now, GMToff) < execute_time)
		return;
    dj = first->list;
    while (dj != NULL) {
        do_command(dj);
        dj = dj->next; 
    }
    config->cron = first->next;
    first->next = NULL;

    /* add job executed to the queue again */
    aj = first->list;
    first->list = NULL;
    ps_free(first);
    
    while (aj != NULL) {
        p = aj->next;
        //refresh_job(aj);
        add_job(config, aj, execute_time);
        aj = p;
    }
}

void
desplay_time(time_st *tm, char *type)
{
	int i = 0;
	while (i < TIME_MAP_LEN-1) {
		if ((unsigned long)1 & (tm->value >> i)) {
			printf("%s_list:%d\n", type, i);
		}
		i++;
	}
	//printf("%s_gap:%d\n", type, tm->gap);
}

//void
joblist_t *
createjob_by_arr(char (*arr)[PS_MAXCMD], int offline)
{
	joblist_t *job;
	
    job = (joblist_t*)ps_malloc(sizeof(joblist_t));

    job->min = time_format(arr[1], TIME_TYPE_MIN);
    job->hour = time_format(arr[2], TIME_TYPE_HOUR);
    job->day = time_format(arr[3], TIME_TYPE_DAY);
    job->mon = time_format(arr[4], TIME_TYPE_MON);
    job->week = time_format(arr[5], TIME_TYPE_WEEK);
    strcpy(job->command, arr[6]);

    job->time_out = 8;
    job->mem_limit = 800;

    job->offline = offline;
    job->jobid = ++MaxJobId;

	if (DebugFlags == 1) {
		desplay_time(&job->min, "min");
		desplay_time(&job->hour, "hour");
		desplay_time(&job->day, "day");
		desplay_time(&job->week, "week");
		desplay_time(&job->mon, "mon");
		printf("command:%s\n", job->command);
		printf("offlien:%d\n", job->offline);
		printf("jobid:%d\n", job->jobid);
	}

	return job;
}

void
free_joblist(joblist_t *job)
{
    joblist_t *j, *nj;
    j = job;
    while (NULL != j) {
        nj = j->next;
        free_joblist(j->child);
        j->next = NULL;
        free_job_node(j);
        j = nj;
    }
}

void
free_job_node(joblist_t *node)
{
    node->child = NULL;
    ps_free(node->jobname);
    ps_free(node->responsible);
    ps_free(node->mails);
    ps_free(node->expression);
    ps_free(node->command);
    ps_free(node->parents);
    ps_free(node->child);
}

void
split_one(char term, char *s, char a[], char b[])
{
	int i = 0;
	int flag = 0;
	char *p = s;

	while (*p != '\0') {
		if (*p == term) {
			a[i] = '\0';
			i = 0;	
			flag = 1;
			p++;
			continue;
		}
		if (flag == 0) {
			a[i++] = *p;
		} else {
			b[i++] = *p;
		}
		p++;
	}
	b[i] = '\0';
}

int
in_str(char term, char *s)
{
	int ret = 0;
	char *p = s;
	while (*p != '\0') {
		if (*p == term) {
			ret = 1;
			break;
		}
		p++;
	}
	return ret;
}

void
unit_format(unsigned long *value, char *str, int type)
{
    unsigned long tmp = 0;
    int gap = 0;
    char h[100], t[100];

    memset(h, '\0', sizeof(h));
    memset(t, '\0', sizeof(t));

	if (in_str('/', str)) {
		split_one('/', str, h, t);
		gap = atoi(t);
    } else {
		strcpy(h, str);
    }

	if (in_str('-', h)) {
		int i;
		char a[10], b[10];
		split_one('-', h, a, b);
        if (!validate_time_range(atoi(a), atoi(b), type)) {
            *value = TIME_FORM_BAD;
            return;
        }
		i = atoi(a);
		while (i <= atoi(b)) {
			tmp |= (unsigned long)1 << i++;
		}
	} else {
        if (in_str('*', h)) {
            int i = get_time_min(type);
            while (i <= get_time_max(type)) {
                tmp |= (unsigned long)1 << i++;
            }
        } else {
            if (!validate_time_equal(atoi(h), type)) {
                *value = TIME_FORM_BAD;
                return;
            }
            tmp |= (unsigned long)1 << atoi(h);
        }
	}

    if (gap > 0) {
        int i, is_first = 0, step = 1;
        i = get_time_min(type);
        while (i <= get_time_max(type)) {
            if ((tmp >> i) & (unsigned long)1) {
                *value |= (unsigned long)1 << i;
                step = gap;
            }
            i += step;
        }
    } else {
        *value |= tmp;
    }
}

int
get_time_min(int type)
{
    switch (type) {
        case TIME_TYPE_MIN:
            return 0;
            break;
        case TIME_TYPE_HOUR:
            return 0;
            break;
        case TIME_TYPE_DAY:
            return 1; 
            break;
        case TIME_TYPE_WEEK:
            return 0; 
            break;
        case TIME_TYPE_MON:
            return 1; 
            break;
        default:
            return 0;
    }
}

int
get_time_max(int type)
{
    switch (type) {
        case TIME_TYPE_MIN:
            return MINUTES_PER_HOUR-1; 
            break;
        case TIME_TYPE_HOUR:
            return HOURS_PER_DAY-1; 
            break;
        case TIME_TYPE_DAY:
            return 31; 
            break;
        case TIME_TYPE_WEEK:
            return DAYS_PER_WEEK-1; 
            break;
        case TIME_TYPE_MON:
            return MONS_PER_YEAR; 
            break;
        default:
            return 0;
    }
}

int
validate_time_equal(int n, int type)
{
    int min = 0, max = 0;
    min = get_time_min(type);
    max = get_time_max(type);
    if (n < min || n > max) {
        return PS_FALSE;
    } else {
        return PS_TRUE;
    }
}

int
validate_time_range(int a, int b, int type)
{
    int max = 0;
    if (!validate_time_equal(a, type) || !validate_time_equal(b, type) || a >= b) {
        return PS_FALSE;
    } else {
        return PS_TRUE;
    }
}

time_st
time_format(char *str, int type)
{
	unsigned long v = 0;
	//char h[100], t[100];
	time_st *tm;
	tm = (time_st*)malloc(sizeof(time_st));
	//memset(h, '\0', sizeof(h));
	//memset(t, '\0', sizeof(t));
	
    if (in_str(',', str)) {
       char *p;
       char q[10];
       int i = 0;
       p = str;
       memset(q, '\0', sizeof(q));

       while (*p != '\0') {
           if (*p == ',') {
               q[i] = '\0';
               printf("douhao-------:%s\n", q);
               unit_format(&v, q, type); 
               memset(q, '\0', sizeof(q));
               i = 0;
           } else {
               q[i++] = *p;
           }
           p++;
       }
       printf("douhao-------:%s\n", q);
       unit_format(&v, q, type); 
       memset(q, '\0', sizeof(q));
    } else {
        unit_format(&v, str, type); 
    }
    	
	tm->value = v;
	tm->type = type;
	return *tm;
}
