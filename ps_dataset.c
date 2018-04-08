#include <stdlib.h>
#include <string.h>
#include "ps_dataset.h"

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
    config->cron = NULL;
    config->m_time = get_current_time();
}

void
load_file_data(config_t *old_config)
{
    struct stat statbuf;
    DIR *dir;
    struct dirent *dp;
    config_t *new_config;

    if (stat(Configdir, &statbuf) < PS_OK) {
        _exit(PS_FAILURE);
    }

    /* don't need to update config if the cron-dir has not changed */
    if(old_config->mtime == statbuf.st_mtime) {
        return;
    }

    new_config->cron = NULL;
    new_config->mtime = statbuf.st_mtime;

    if (!(dir = opendir(Configdir))) {
        _exit(PS_FAILURE);
    }

    while (NULL != (dp = readdir(dir))) {
        char fname[MAXNAMLEN+1], tabname[MAXNAMLEN+1]; 
        if (dp->d_name[0] == '.')
            continue;
        if (strlen(dp->d_name) >= sizeof fname)
            continue;
        if (0 != strcmp(db->db, CRONFILE))
            continue;
        (void) strcpy(fname, dp->d_name);
        if (!glue_strings(tabname, sizeof tabname, SPOOL_DIR,
                    fname, '/'));
        process_crontab(tabname, &statbuf, &new_config, old_config);
    }
    closedir(dir);
}

void
load_config(config_t *old_config)
{
    int mysql_ok = 0;

    if (*old_queue->type == PS_CONFIG_MYSQL) {
        // TODO check and load mysql config
        mysql_ok = 1; 
    }
    if (mysql_ok == 0) {
        // TODO check and load file config
        load_file_data(&old_config);
    }
}

static void
process_crontab(const char *tabname, struct stat *statbuf, \
        config_t *old_config)
{
    int crontab_fd = PS_OK - 1 ;
    cronqueue_t *cron_list = NULL;

    if ((crontab_fd = open(tabname, O_RDONLY|O_NONBLOCK|O_NOFOLLOW, 0)) < PS_OK) {
        goto better_return;
    }
    if (fstat(crontab_fd, statbuf) < PS_OK) {
        goto better_return;
    }
    if ((statbuf->st_mode & 07777) != 0600) {
        goto better_return;
    }
    unlink_cron(&old_config);
    load_cron(&old_config, crontab_fd, statbuf.st_mtime);
better_return:
    if (crontab_fd >= PS_OK) {
        close(crontab_fd);
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
load_cron(config_t *config, int crontab_fd, time_t mtime)
{
    cronqueue_t *cron;
    FILE *file;

    if (!(file = fdopen(crontab_fd, "r")))
        _exit(PS_FAILURE);

    // TODO load cron
}
