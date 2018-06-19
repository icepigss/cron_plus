#ifndef _H_PS_FUNCS_
#define _H_PS_FUNCS_

#include "ps_globals.h"
#include "ps_structs.h"

void   cron_sleep1(int []),
	   handle_request(char []),
	   init_env(),
       regist_signal(),
       parse_args(int, int **),
       acquire_daemonlock(int),
       free_cronlist(cronqueue_t *),
	   init_config(config_t *),
	   load_config(config_t *),
	   load_file_data(config_t *),
	   unlink_cron(config_t *),
	   load_cron(config_t *, int, time_t),
	   exit_job_ps(),
	   job_callback(joblist_t *, int),
	   shut_down_ex(int),
	   set_sig_callback(int),
	   child_process(joblist_t *),
	   do_command(joblist_t *),
	   refresh_job(joblist_t *),
	   add_job(config_t *, joblist_t *, int),
	   delete_job(cronqueue_t *, joblist_t *),
	   run_job(config_t *),
	   free_job_node(joblist_t *),
	   free_joblist(joblist_t *),
	   ps_write_stdout(char *),
	   ps_write_stderr(char *),
	   ps_free(void *),
	   build_tcp_server(int []),
	   ps_file_log(char *, int),
	   find_job(joblist_t *, cronqueue_t *, int),
	   unit_format(unsigned long *, char *),
	   split_one(char, char *, char *, char *),
	   display_config(config_t *);

void *
	   ps_malloc(size_t),
	   pv_calloc(size_t, size_t);
	   //ps_realloc(void *, size_t),
	   //ps_copy_str(const char *);


static void
	   set_time(int),
	   cron_sleep(int [], int),
	   ps_show_version_info(),
	   process_crontab(const char *, struct stat *, config_t *);

int    s_get_options(int, char *const *),
	   get_job_mem(const pid_t),
	   get_char(FILE *),
	   preg_match_cmd(char *, char (*)[PS_MAXCMD]),
	   glue_strings(char *, size_t, const char *, const char *, char),
	   in_str(char, char *);

char *	trim(char [], char *);

time_t  get_current_time();

unsigned long 
	   gettime_by_format(time_st *, time_st *, time_st *, time_st *, time_st *, int);
time_st
		time_format(char *str);

joblist_t *
	   createjob_by_arr(char (*)[PS_MAXCMD], int);

#endif
