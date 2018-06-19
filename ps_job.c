#include "ps_cron.h"

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
	printf("working ending:%d\n", signo);
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
					printf("prprprprpr:%d\n", pr);
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

unsigned long
gettime_by_format(time_st *min, time_st *hour, 
		time_st *day, time_st *week, time_st *mon, int last_execute_time)
{
	int job_min, job_hour, job_day, job_week, job_mon;
	time_t now;
	struct tm *timenow;
	unsigned long next_min;
	int over_sec;
	int GMToff;

	time(&now);
	timenow = localtime(&now);
	GMToff = get_gmtoff(&now, timenow);
	
	next_min = (now + GMToff) / (time_t)SECONDS_PER_MINUTE + 1;
	over_sec = (now + GMToff) % (time_t)SECONDS_PER_MINUTE;

	/*
	 * Leave enough time for program processing
	 */
	if (over_sec > SECONDS_PER_MINUTE/2 && last_execute_time == 0) {
		next_min++;
	}

	job_min = timenow->tm_min+1;
	job_hour = timenow->tm_hour;
	job_day = timenow->tm_mday;
	job_week = timenow->tm_wday;
	job_mon = timenow->tm_mon;

	if (min->value == 0 && hour->value == 0 && day->value == 0  
			&& week->value == 0 && mon->value == 0) {
		return next_min;
	}
    if (min->value >> TIME_MAP_LEN-3 == 1) {
    }
	// TODO time parse

	return 36464554;
}

void 
add_job(config_t *config, joblist_t *job, int last_execute_time)
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
    joblist_t *dj, *aj = NULL;
	int execute_time;
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
	if ((now + GMToff) / (time_t)SECONDS_PER_MINUTE > execute_time)
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
        //refresh_job(aj);
        add_job(config, aj, execute_time);
        aj = aj->next;
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
	printf("%s_gap:%d\n", type, tm->gap);
}

//void
joblist_t *
createjob_by_arr(char (*arr)[PS_MAXCMD], int offline)
{
	joblist_t *job;
	
    job = (joblist_t*)ps_malloc(sizeof(joblist_t));

    job->min = time_format(arr[1]);
    job->hour = time_format(arr[2]);
    job->day = time_format(arr[3]);
    job->week = time_format(arr[4]);
    job->mon = time_format(arr[5]);
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
unit_format(unsigned long *value, char *str)
{
	if (in_str('-', str)) {
		int i;
		char a[10], b[10];
		split_one('-', str, a, b);
		i = atoi(a);
		while (i <= atoi(b)) {
			*value |= (unsigned long)1 << Tran_Zero(i);
			i++;
		}
	} else {
		*value |= (unsigned long)1 << Tran_Zero(atoi(str));
	}
}

time_st
time_format(char *str)
{
	int g = 0;
	unsigned long v = 0;
	char h[100], t[100];
	time_st *tm;
	tm = (time_st*)malloc(sizeof(time_st));
	memset(h, '\0', sizeof(h));
	memset(t, '\0', sizeof(t));
	
	if (in_str('/', str)) {
		int i = 0;
		char *p;

		split_one('/', str, h, t);
		g = atoi(t);
	} else {
		strcpy(h, str);
	}


	if (in_str('*', h)) {
		v = 0;
	} else if (in_str(',', h)) {
		char *p;
		char q[10];
		int i = 0;
		p = h;
		memset(q, '\0', sizeof(q));

		while (*p != '\0') {
			if (*p == ',') {
				q[i] = '\0';
				unit_format(&v, q);
				memset(q, '\0', sizeof(q));
				i = 0;
			} else {
				q[i++] = *p;
			}
			p++;
		}
		q[i] = '\0';
		unit_format(&v, q);
	} else if (in_str('-', h)) {
		int i;
		char a[10], b[10];
		split_one('-', h, a, b);
		i = atoi(a);
		while (i <= atoi(b)) {
			v |= (unsigned long)1 << Tran_Zero(i);
			i++;
		}
	} else {
		v = (unsigned long)1 << Tran_Zero(atoi(h));
	}

	tm->value = v;
	tm->gap = g;
	return *tm;
}
