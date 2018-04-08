#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include "ps_job.h"

void
exit_job_ps()
{
    _exit(PS_SUCCESS);
}

void
job_callback(int signo)
{
    // TODO callback
}

void
shut_down_ex(int signo)
{
    job_callback(signo);
    _exit(PS_FAILURE);
}

void
set_sig_callback(ps_int_t time_out)
{
    if (signal(SIGALRM, shut_down_ex) == SIG_ERR)
        _exit(PS_FAILURE);        
    if (signal(SIGABRT, shut_down_ex) == SIG_ERR)
        _exit(PS_FAILURE);        
    if (signal(SIGUSR2, shut_down_ex) == SIG_ERR)
        _exit(PS_FAILURE);        
    alarm(time_out);
    atexit(job_callback);
}

void
child_process(joblist_t *job)
{
    pid_t pid;

    pid = fork();
    switch (pid) {
        case -1:
            _exit(PS_FAILURE);
            break;
        case 0:
            set_sig_callback(job->time_out);
            break;
        default:
            ps_int_t mem_usage;
            if (signal(SIGCHLD, exit_job_ps) == SIG_ERR)
                _exit(PS_FAILURE);

            while (PS_TRUE) {
                sleep(1);
                mem_usage = get_job_mem(pid);
                if (mem_usage > job->mem_limit) {
                    kill(pid, SIGUSR2);
                }
            }
            break;
    }
}

void
do_command(joblist_t *job)
{
    switch (fork()) {
        case -1:
            _exit(PS_FAILURE);
            break;
        case 0:
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
add_job(cronqueue_t *cron_queue, joblist_t *job)
{
}

joblist_t *
find_job(cronqueue_t *cron_queue, ps_int_t jobid)
{
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
run_job(cronqueue_t *cron_queue)
{
    cronqueue_t *first = NULL;
    joblist_t *dj, aj = NULL;

    if (cron_queue->list == NULL) {
        return;
    }
    first = reserve = cron_queue;
    dj = first->list;
    while (dj != NULL) {
        do_command(dj);
        dj = dj->next; 
    }
    cron_queue = first->next;
    first->next = NULL;

    /* add job executed to the queue again */
    aj = first->list;
    first->list = NULL;
    ps_free(first);
    
    while (aj != NULL) {
        refresh_job(&aj);
        add_job(&cron_queue, aj);
        aj = aj->next;
    }
}

void
free_joblist(joblist_t *job)
{
    joblist_t *j *nj;
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
free_job_node(cronqueue_t *node)
{
    node->child = NULL;
    free(node->jobname);
    free(node->responsible);
    free(node->mails);
    free(node->expression);
    free(node->command);
    free(node->parents);
    free(node->child);
}
