#include "ps_cron.h"

static char* ps_etc_suffix = NULL;
static int ps_show_help;
static int ps_show_version;
static int ps_daemon_flag;
static int timeRunning, virtualTime, clockTime;
static long GMToff;
static  volatile sig_atomic_t   got_sighup, got_sigchld;

enum timejump { negative, small, medium, large };

/**
 * sleep 60 seconds,
 * also accept request
 */
void
cron_sleep1(int pfd[])
{
    int i = 0;
    char line[PS_MAXLINE];

    while (i++ < 60) {
        /** check if there is a request **/
        if (read(pfd[0], &line, 1024) == 1) {
            handle_request(line);
        }
        sleep(1);
    }
}

static void
cron_sleep(int pfd[], int target) {
    char line[PS_MAXLINE];
    time_t t1, t2;
    int seconds_to_wait;

    t1 = time(NULL) + GMToff;
    seconds_to_wait = (int)(target * SECONDS_PER_MINUTE - t1) + 1;
	if (DebugFlags == 1) {
		fprintf(stdout, "seconds_to_wait:%d\n", seconds_to_wait);
	}

    while (seconds_to_wait > 0 && seconds_to_wait < 65) {
		/*
		 * Sleep to wait until next minute, 
		 * also check the request every second
		 */
        sleep(1);

		/*
		 * In order to accurately time to the next minte,
		 * it will not process any request if there is
		 * not much time left(less than 5 senconds)
		 */
		if (seconds_to_wait > 5) {
			int len;
			do {
				len = read(pfd[0], &line, PS_MAXLINE);
				if (len > 0) {
					if (DebugFlags == 1) {
						fprintf(stdout, "recieve(%d):%s", len, line);
					}
					handle_request(line);
				}
			} while (len > 0);
		}

        t2 = time(NULL) + GMToff;
        seconds_to_wait -= (int)(t2 - t1);
        t1 = t2;
    }
}

/**
 * parse master's request(such as adding job,deleting job),
 * then execute related commands
 */
void
handle_request(char request[])
{
	if (DebugFlags == 1) {	
		fprintf(stdout, "handle_request\n");
	}
}

void
sigchld_handler(int x) {
	got_sigchld = 1;
}

void
regist_signal()
{
	struct sigaction sact;

	sact.sa_flags = 0;
	sact.sa_handler = sigchld_handler;
	(void) sigaction(SIGCHLD, &sact, NULL);
	//sact.sa_handler = sighup_handler;
	//(void) sigaction(SIGHUP, &sact, NULL);
	//sact.sa_handler = quit;
	//(void) sigaction(SIGINT, &sact, NULL);
	//(void) sigaction(SIGTERM, &sact, NULL);
}

static void
sigchld_reaper(void) {
	int waiter;
	pid_t pid;
	printf("outoutoutoutout\n");

	do {
		pid = waitpid(-1, &waiter, WNOHANG);
		printf("ssssssssssss:%d\n", waiter);
		switch (pid) {
			case -1:
				if (errno == EINTR)
					continue;
					break;
			case 0:
					break;
			default:
					break;
		}
	} while (pid > 0);
}

void
free_cronlist(cronqueue_t *cron)
{
    cronqueue_t *c, *nc;
    c = cron;
    while (NULL != c) {
        nc = c->next;
        c->next = NULL;
        free_joblist(c->list);
        c = nc;
    }
}

static int
ps_get_options(int argc, char *const *argv)
{
    char *p;
    int i;

    for (i = 1; i < argc; i++) {
        p = (char *) argv[i];
        if (*p++ != '-') {
            ps_write_stderr("invalid option");
            return PS_ERROR;
        }

        while (*p) {
            switch (*p++) {
                case '?':
                case 'h':
                    ps_show_version = 1;
                    ps_show_help = 1;
                    break;
                case 'v':
                case 'V':
                    ps_show_version = 1;
                    break;
                case 'c':
                    if (*p) {
                        ps_etc_suffix = p;
                        goto next;
                    }
                    if (argv[++i]) {
                        ps_etc_suffix = (char *) argv[i];
                        goto next;
                    }
                    break;
                case 'd':
                    ps_daemon_flag = 1;
                    break;
                case 'D':
                    DebugFlags = 1;
                    break;
                case 't':
                    if (*p) {
                        ConfigMode = *p == 'm' ? PS_CONFIG_MYSQL : PS_CONFIG_FILE;
                    } else {
                        ConfigMode = PS_CONFIG_FILE;
                    }
                    goto next;
                default:
                    ps_write_stderr("invalid option");
                    return PS_ERROR;
            }
        }
next:
        continue;
    }

    return PS_OK;
}

static void
ps_show_version_info()
{
    ps_write_stderr("cron_plus version: " PS_VERSION PS_LINEFEED);

    if (ps_show_help) {
        ps_write_stderr(
                "Usage: pcron [-?hvVtd] [-s signal] " PS_LINEFEED
                "Options:" PS_LINEFEED
                "  -?,-h         : show help" PS_LINEFEED
                "  -v,-V         : show version" PS_LINEFEED
                "  -c            : set file config dir: "
                "  -t type       : set config-type: "
                                   "mysql[m], file[f]" PS_LINEFEED
                "  -d            : set daemon mode" PS_LINEFEED
                "  -D            : debug mode" PS_LINEFEED PS_LINEFEED
                );
    }
}

void
init_env()
{
	char *h;
	char f[50];

	h = getenv("HOME");

    ConfigMode = PS_CONFIG_FILE;
    DebugFlags = 0;
    ps_daemon_flag = 0;

    memset(EtcDir, '\0', sizeof(EtcDir));
    snprintf(EtcDir, sizeof(EtcDir), "%s/%s", getenv("HOME"), ETC_DIR);

	memset(f, '\0', sizeof(f));
	snprintf(f, sizeof(f), "%s/%s", h, WORK_DIR);
	mkdir(f, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

	chdir(f);

	memset(f, '\0', sizeof(f));
	snprintf(f, sizeof(f), "%s/%s", h, VAR_DIR);
	mkdir(f, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

    /*
	memset(f, '\0', sizeof(f));
	snprintf(f, sizeof(f), "%s/%s", h, ETC_DIR);
	mkdir(f, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    */

	memset(f, '\0', sizeof(f));
	snprintf(f, sizeof(f), "%s/%s", h, LOG_DIR);
	mkdir(f, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

	memset(f, '\0', sizeof(f));
	snprintf(f, sizeof(f), "%s/%s", h, JOB_DIR);
	mkdir(f, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

}

static void
set_time(int initialize) {
    struct tm tm;
    static int isdst;

    StartTime = time(NULL);

    /* We adjust the time to GMT so we can catch DST changes. */
    tm = *localtime(&StartTime);
    if (initialize || tm.tm_isdst != isdst) {
        isdst = tm.tm_isdst;
        GMToff = get_gmtoff(&StartTime, &tm);
    }
    clockTime = (StartTime + GMToff) / (time_t) SECONDS_PER_MINUTE;
	if (DebugFlags == 1) {
		//fprintf(stdout, "set_time\nStartTime:%lu\tGMToff:%lu\tclockTime:%d\n", StartTime, GMToff,clockTime);
	}
}

int
main(int argc, char *argv[])
{
    int     pfd[2];
    int     fd;
    pid_t   pid;

    if (pipe(pfd) < 0)
        _exit(PS_FAILURE);

    config_t config;
    cronqueue_t *cron_queue;

	ProgramName = argv[0];

	init_env();

    if (PS_ERROR == ps_get_options(argc, argv)) {
        return PS_ERROR;
    }

    if (ps_etc_suffix != NULL) {
        memset(EtcDir, '\0', sizeof(EtcDir));
        snprintf(EtcDir, sizeof(EtcDir), "%s", ps_etc_suffix);
    }
    //printf("afasfsfsf:%s\n", EtcDir);_exit(-1);

    if (ps_show_version) {
        ps_show_version_info();
    }

    /* daemon model */
    if (ps_daemon_flag == 1) {
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


    acquire_daemonlock(0);

    init_config(&config);
    load_config(&config);

    cron_queue = config.cron;

	//char    line[1000];

    switch (pid = fork()) {
        case -1:
            _exit(PS_FAILURE);
        case 0:
			regist_signal();
			signal(SIGCHLD, SIG_IGN);
            close(pfd[1]);
			int flag = fcntl(pfd[0], F_GETFL, 0);
			flag |= O_NONBLOCK;
			fcntl(pfd[0], F_SETFL, flag);

			set_time(PS_TRUE);

            while (PS_TRUE) {
				timeRunning = virtualTime = clockTime;
				int timeDiff;
				//enum timejump wakeupKind;

				do {
					cron_sleep(pfd, timeRunning + 1);
					set_time(PS_FALSE);
				} while (clockTime == timeRunning);
				timeRunning = clockTime;

				timeDiff = timeRunning - virtualTime;

				if (DebugFlags == 1) {
					fprintf(stdout, "timediff:%d\n", timeDiff);
				}
                run_job(&config);
				if (DebugFlags == 1) {
					display_config(&config);
				}
                if (ps_daemon_flag == 1) {
                    display_config_log(&config);
                }
                //load_config(&config);
                //cron_queue = config->cron;
				//int waiter;
				//while ((pid = wait(&waiter)) < PS_OK && errno == EINTR)
				//	;
				if (got_sigchld) {
					got_sigchld = 0;
					sigchld_reaper();
				}
            }
            break;
        default:
            close(pfd[0]);
            build_tcp_server(pfd);
            if (DebugFlags == 1) {
                fprintf(stdout, "tcp server \n");
            }
            break;
    }
    _exit(PS_FAILURE);
}
