cron_plus
===============

A new crond server on linux, it is like linux's crond, but has more features:
- APIs for external access
- Support dependent task
- Capture task status
- Check history log
- Config task owner
- Support subscription and monitoring alarm

------
### Usage
$./pcron -h
cron_plus version: 1.0
Usage: pcron [-?hvVtd] [-s signal] 
Options:
  -?,-h         : this help
  -v,-V         : show version and exit
  -t type       : set config-type: mysql[m], file[f]
  -d            : set daemon mode

  -D            : debug mode

### Configuration format
- Just like crontab
0 * * * * /usr/bin/sh  /home/work/job/test.sh 2>&1 &

### Extended
- You can create a prety website to manage your tasks
- You can also use it as a substitue for crond

### APIs
Upcoming updates
