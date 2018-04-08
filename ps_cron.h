#ifndef _H_PS_CRON_
#define _H_PS_CRON_

#include "ps_globals.h"
#include "ps_job.h"

typedef struct cronqueue_s cronqueue_t;

static ps_int_t Daemonflag = 0;

/* cron queue */
struct cronqueue_s {
    cronqueue_t     *next;
    joblist_t       *list; /* point to current joblist */
    time_t          timestamp; /* current execution time */
};

void   cron_sleep(),
       regist_signal(),
       parse_args(int, int **),
       free_cronlist(cronqueue_t *);

#endif
