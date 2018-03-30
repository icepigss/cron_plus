#ifndef _H_PS_DATASET_
#define _H_PS_DATASET_

#include "globals.h"

typedef struct config_s config_t;

struct config_s {
    ps_int_t      type; /* data source type（file or MySQL） */
    time_t      m_time; /* last modify time */
};

void    init_queue(cronqueue_t *),
        load_config(cronqueue_t *);

time_t  get_current_time();

#endif
