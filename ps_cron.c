#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include "ps_cron.h"
#include "ps_job.h"

/**
 * sleep 60 seconds,
 * also accept request
 */
void
cron_sleep(int pfd[])
{
    int i = 0;
    char line[PS_MAXLINE];

    while (i++ < 60) {
        /** check if there is a request **/
        if (read(fd[0], &line, 1024) == 1) {
            handle_request(line);
        }
        sleep(1);
    }
}

/**
 * parse master's request(such as adding job,deleting job),
 * then execute related commands
 */
void
handle_request(char request[])
{
}

void
regist_signal()
{
}

void
free_cronlist(cronqueue_t *cron)
{
    cronqueue_t *c *nc;
    c = cron;
    while (NULL != c) {
        nc = c->next;
        c->next = NULL;
        free_joblist(c->list);
        c = nc;
    }
}

int 
main(int argc, int *argv[])
{
    int     pfd[2];
    int     fd;
    pid_t   pid;

    if (pipe(pfd) < 0)
        _exit(PS_FAILURE);

    config_t config;
    cron_queue *cron_queue;

    parse_args(argc, argv);

    init_config(&config);
    load_config(&config);

    cron_queue = config->cron;

    /* daemon model */
    if (Daemonflag == PS_FALSE) {
        switch (fork()) {
            case -1:
                break;
            case 0:
                (void) setsid();
                if ((fd = open(_PATH_DEVNULL, O_RDWR, 0)) >= 0) {
                    (void) dup2(fd, STDIN);
                    (void) dup2(fd, STDOUT);
                    (void) dup2(fd, STDERR);
                    if (fd != STDERR)
                        (void) close(fd);
                }
                break;
            default:
                _exit(0);
        }
    }

    switch (pid = fork()) {
        case -1:
            _exit(PS_FAILURE);
        case 0:
            close(pfd[1]);
            while (PS_TRUE) {
                cron_sleep(pfd); /* block to wait 1 minute */
                run_job(&cron_queue);
                //load_config(&config);
                //cron_queue = config->cron;
            }
            break;
        default:
            close(pfd[0]);
            build_tcp_server(pfd);
            break;
    }
    _exit(PS_FAILURE);
}
