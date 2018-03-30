#include <stdlib.h>
#include <string.h>
#include "ps_config.h"
#include "ps_cron.h"
#include "ps_malloc.c"

time_t
get_current_time()
{
    time_t timep;  
    time(&timep);
    return timep;
}

void 
init_queue(cronqueue_t *cron_queue)
{
    cron_queue = (cronqueue_t*)ps_malloc(sizeof(cronqueue_t)); 
    if (cron_queue== NULL) {
        exit(ERROR_EXIT);
    }
    cron_queue->next = NULL;
    cron_queue->list = NULL;
    cron_queue->timestamp = get_current_time();
}

void
load_config(cronqueue_t *old_queue)
{
    int mysql_ok = 0;
    cronqueue_t * new_queue; 

    if (*old_queue->type == PS_CONFIG_MYSQL) {
        // TODO check and load mysql config
        mysql_ok = 1; 
    }
    if (mysql_ok == 0) {
        // TODO check and load file config
    }

    *old_queue = new_queue;
}
