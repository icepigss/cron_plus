#ifndef _H_PS_JOB_
#define _H_PS_JOB_

#include "ps_globals.h"

typedef struct joblist_s joblist_t;

/* job node */
struct joblist_s {
    joblist_t       *next;
    joblist_t       *child; /* point to child list */
    time_t          execute_time; /* start execution time */
    ps_int_t        time_out; /* process will terminate while exceeding time (second) */
    ps_int_t        mem_limit; /* warning will be send while exceeding maximum memory limit */
    ps_int_t        jobid;
    ps_char_t       *jobname;
    ps_char_t       *responsible;
    ps_char_t       *mails;
    ps_char_t       *expression; /* eg: 1 * * * * */
    ps_char_t       *command; /* to be executing */
    ps_char_t       *parents; /* save ids of parents */

    struct bs { /* two bytes for some flag */
        unsigned    f1:2; /* is there a child */
        unsigned    f2:2; /* is there a parent */
        unsigned    f3:4; /* retry times */
        unsigned    f4:8; /* numbers of subprocess */
    };
};

void    exit_job_ps(),
        job_callback(int),
        shut_down_ex(int),
        set_sig_callback(ps_int_t),
        child_process(joblist_t *),
        do_command(joblist_t *),
        refresh_job(joblist_t *),
        add_job(joblist_t *),
        delete_job(cronqueue_t * joblist_t *),
        run_job(cronqueue_t *),
        free_job_node(joblist_t *,
        free_joblist(joblist_t *);

joblist_t *
        find_job(cronqueue_t *, ps_int_t);

#endif
