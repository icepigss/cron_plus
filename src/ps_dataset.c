#include "ps_cron.h"

time_t
get_current_time()
{
    time_t timep;  
    time(&timep);
    return timep;
}

void 
init_config(config_t *config)
{
    config = (config_t*)malloc(sizeof(config_t));
    config->cron = NULL;
    config->mtime = get_current_time();
}

void
load_file_data(config_t *old_config)
{
    struct stat statbuf;
    //char configDir[100];
    DIR *dir;
    struct dirent *dp;
    //config_t *new_config;

    //snprintf(configDir, sizeof(configDir), "%s/%s", getenv("HOME"), ETC_DIR);
    //if (stat(configDir, &statbuf) < 0) {
    if (stat(EtcDir, &statbuf) < 0) {
        _exit(PS_FAILURE);
    }

    /* don't need to update config if the cron-dir has not changed */
    if(old_config->mtime == statbuf.st_mtime) {
        //return;
    }

    old_config->mtime = statbuf.st_mtime;

    if (!(dir = opendir(EtcDir))) {
        char buf[100];
        memset(buf, '\0', sizeof(buf));
        sprintf(buf, "%s: can not open dir (%s)", ProgramName, strerror(errno));
        ps_write_stderr(buf);
        ps_file_log(buf, 0);
        _exit(PS_FAILURE);
    }

    while (NULL != (dp = readdir(dir))) {
        char fname[MAXNAMLEN+1], tabname[MAXNAMLEN+1];
        if(strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0) 
            continue;
        if (strlen(dp->d_name) >= sizeof fname)
            continue;
        if (0 != strcmp(dp->d_name, PS_CRON_FILE))
            continue;
        (void) strcpy(fname, dp->d_name);

        if (DebugFlags == 1) {
            fprintf(stdout, "config file: %s\n", fname);
        }
        snprintf(tabname, sizeof(tabname), "%s/%s/%s", getenv("HOME"), ETC_DIR, fname);
        process_crontab(tabname, &statbuf, old_config);
    }
    closedir(dir);
}

void
load_config(config_t *old_config)
{
    int mysql_ok = 0;

    if (PS_CONFIG_MYSQL == ConfigMode) {
        // TODO check and load mysql config
        mysql_ok = 1; 
    }
    if (mysql_ok == 0) {
        // check and load file config
        load_file_data(old_config);
    }
}

static void
process_crontab(const char *tabname, struct stat *statbuf, \
        config_t *old_config)
{
    int crontab_fd = -1 ;
    cronqueue_t *cron_list = NULL;

    if ((crontab_fd = open(tabname, O_RDONLY|O_NONBLOCK|O_NOFOLLOW, 0)) < 0) {
        goto better_return;
    }
    if (fstat(crontab_fd, statbuf) < 0) {
        goto better_return;
    }
    if ((statbuf->st_mode & 07777) != 0600) {
        goto better_return;
    }
    unlink_cron(old_config);
    load_cron(old_config, crontab_fd, statbuf->st_mtime);
better_return:
    if (crontab_fd >= 0) {
        if (DebugFlags == 1) {
            fprintf(stdout, "%s load done\n", tabname);
        }
        close(crontab_fd);
    } else {
        if (DebugFlags == 1) {
            fprintf(stdout, "%s load failed\n", tabname);
        }
    }
}

void
unlink_cron(config_t *config)
{
    cronqueue_t *cron;

    cron = config->cron;
    config->cron = NULL;
    free_cronlist(cron);
}

void
display_config_log(config_t *config)
{
    int i = 1;
    int j = 1;
	int GMToff;
    char buf[1000];
	time_t now;
	struct tm *timenow;
	unsigned long current_min;
    cronqueue_t *p = config->cron;
    if (p == NULL) {
        return;
    }
    joblist_t *job;

	time(&now);
	timenow = localtime(&now);
	GMToff = get_gmtoff(&now, timenow);

    current_min = Time_To_Min(now, GMToff);

    memset(buf, '\0', sizeof(buf));
    sprintf(buf, "\n-------------\ncurrent_min:%lu\n", current_min);
    ps_file_debug(buf);
    memset(buf, '\0', sizeof(buf));
    while (p != NULL) {
        sprintf(buf, "cron:%d:time:%lu\n", i++, p->timeframe);
        ps_file_debug(buf);
        memset(buf, '\0', sizeof(buf));
        job = p->list;
        p = p->next;
        if (job == NULL) continue;
        while (job != NULL) {
            sprintf(buf, "\tjob:%d\n\t\tjobid:%d\n\t\tmin:%lu\n\t\thour:%lu\n\t\tday:%lu\n\t\tmon:%lu\n\t\tcmd:%s\n", j++, job->jobid, job->min.value, job->hour.value, job->day.value, job->mon.value, job->command);
            ps_file_debug(buf);
            memset(buf, '\0', sizeof(buf));
            job = job->next;
        }
    }
}

void
display_config(config_t *config)
{
    int i = 1;
    int j = 1;
    cronqueue_t *p = config->cron;
    if (p == NULL) {
        return;
    }
    joblist_t *job;
    while (p != NULL) {
        printf("cron:%d:time:%lu\n", i++, p->timeframe);
        job = p->list;
        p = p->next;
        if (job == NULL) continue;
        j = 1;
        while (job != NULL) {
            printf("\tjob:%d\n", j++);
            printf("\t\tjobid:%d\n", job->jobid);
            printf("\t\tmin:%lu\n", job->min.value);
            printf("\t\thour:%lu\n", job->hour.value);
            printf("\t\tday:%lu\n", job->day.value);
            printf("\t\tweek:%lu\n", job->week.value);
            printf("\t\tmon:%lu\n", job->mon.value);
            printf("\t\tcmd:%s\n", job->command);
            printf("\t\toffline:%d\n", job->offline);
            job = job->next;
        }
    }
}

void
load_cron(config_t *config, int crontab_fd, time_t mtime)
{
    cronqueue_t *cron;
    FILE *file;
    time_st t;

    char cmd[PS_MAXCMD];
    char line[PS_MAXCMD];
    char arr[10][PS_MAXCMD];
    int offline = 0;
    joblist_t *job;

    // TODO load cron
    int pre, invalid, online, newline, is_online;
    invalid = offline = online = 0;
    newline = 1;

    if (!(file = fdopen(crontab_fd, "r"))) {
        char buf[100];
        memset(buf, '\0', sizeof(buf));
        sprintf(buf, "%s: can not open config file (%s)", ProgramName, strerror(errno));
        ps_write_stderr(buf);
        ps_file_log(buf, 0);
        _exit(PS_FAILURE);
    }

    while (!feof(file)) {
        char *str;
        char first;

        memset(line, '\0', sizeof(line));
        fgets(line, PS_MAXCMD, file);
        str = NULL;
        str = trim(line, " \t");
        first = str[0]; 
        if (first == '#') {
            offline = 1;
        }
        //memset(str, '\0', sizeof(str));
        str = NULL;
        str = trim(line, " #\t");
        for (int i = 0; i < 10; i++) {
            memset(arr[i],'\0', sizeof(str));
        }
        if (preg_match_cmd(str, arr) == PS_ERROR) {
            continue;
        }
        if (DebugFlags == 1) {
            printf("cmd reg_match:\n");
            printf("arr0:%s\n", arr[0]);
            printf("arr1:%s\n", arr[1]);
            printf("arr2:%s\n", arr[2]);
            printf("arr3:%s\n", arr[3]);
            printf("arr4:%s\n", arr[4]);
            printf("arr5:%s\n", arr[5]);
            printf("arr6:%s\n", arr[6]);
        }
        int invalid = 0;
        for (int i = 0; i < 6; i++) {
            if (strlen(arr[i]) == 0) {
                invalid = 1;
                break;
            }
        }
        if (invalid == 1) {
            continue;
        }
        job = createjob_by_arr(arr, offline);
        if ((job->min.value >> 61) & (unsigned long)1 || (job->hour.value >> 61) & (unsigned long)1 \
                || (job->day.value >> 61) & (unsigned long)1 || (job->mon.value >> 61) & (unsigned long)1 \
                || (job->week.value >> 61) & (unsigned long)1) {
            // TODO free job node
            if (DebugFlags == 1) {
                if (job->jobid > 0) {
                    printf("create job faild:%d\t%s\n", job->jobid, job->command);
                }
            }
        } else {
            add_job(config, job, 0);
            if (DebugFlags == 1) {
                if (job->jobid > 0) {
                    printf("create job success:%d\t%s\n", job->jobid, job->command);
                }
            }
        }
    }

    if (DebugFlags == 1) {
        display_config(config);
    }
}
