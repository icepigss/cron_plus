objects	= ps_cron.o ps_job.o ps_dataset.o ps_lib.o \
		  ps_log.o ps_malloc.o ps_server.o

cc = gcc

pcron	:	$(objects)
	cc -o pcron $(objects)

$(objects)	:	ps_cron.h ps_config.h ps_globals.h ps_externs.h

.PHONY : clean

clean :
		rm pcron $(objects)
