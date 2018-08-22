#ifndef _H_PS_STRUCTS_
#define _H_PS_STRUCTS_

#include "ps_globals.h"

typedef struct time_s {
	unsigned long value;
	int type; // 1:min 2:hour 3:day 4:week 5:mon
} time_st;

/* job node */
typedef struct joblist_s {
	struct joblist_s *next;
	struct joblist_s *child; /* point to child list */
	time_t          execute_time; /* start execution time */
	int        time_out; /* process will terminate while exceeding time (second) */
	int        mem_limit; /* warning will be send while exceeding maximum memory limit */
	int        jobid;
	int        offline;
	char       *jobname;
	char       *responsible;
	char       *mails;
	char       *expression; /* eg: 1 * * * * */
	char       command[200]; /* to be executing */
	char       *parents; /* save ids of parents */

	time_st			min;
	time_st			hour;
	time_st			day;
	time_st			week;
	time_st			mon;

	struct bs { /* two bytes for some flag */
		unsigned    f1:2; /* is there a child */
		unsigned    f2:2; /* is there a parent */
		unsigned    f3:4; /* retry times */
		unsigned    f4:8; /* numbers of subprocess */
	} bs_t;
} joblist_t;

typedef struct jobinfo_s {
	int valid;
	int status;
	int offline;
} jobinfo_t;

jobinfo_t jobhash[PS_MAX_JOB];

/* cron queue */
typedef struct cronqueue_s {
	struct cronqueue_s *next;
	joblist_t       *list; /* point to current joblist */
	unsigned long   timeframe; /* current execution mins*/
} cronqueue_t;

typedef struct config_s {
    cronqueue_t *cron; /* cron list link */
    time_t      mtime; /* last modify time */
} config_t;


#endif
