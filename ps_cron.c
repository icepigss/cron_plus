#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include "ps_cron.h"
#include "ps_lib.h"

void
cron_sleep()
{
    sleep(60);
}

void
regist_signal()
{
}

int 
main(int argc, int *argv[])
{
    int fd;
    cronqueue_t cron_queue;

    parse_args(argc, argv);

    init_queue(&cron_queue);
    load_config(&cron_queue);

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

    while (PS_TRUE) {
        cron_sleep(); /* block to wait 1 minute */
        run_job(&cron_queue);
        load_config(&cron_queue);
    }
}
