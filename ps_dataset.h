#ifndef _H_PS_DATASET_
#define _H_PS_DATASET_

#include "ps_cron.h"
#include "ps_globals.h"

static ps_char_t Configdir = "/var/pcron";

typedef struct config_s config_t;

struct config_s {
    cronqueue_t *cron; /* cron list link */
    time_t      mtime; /* last modify time */
};

void    init_config(config_t *),
        load_config(config_t *),
        load_file_data(config_t *),
        unlink_cron(cronqueue_t *),
        load_cron(config_t *, int, time_t);

static void
        process_crontab(const char *, struct stat *, config_t *);

time_t  get_current_time();

#endif
