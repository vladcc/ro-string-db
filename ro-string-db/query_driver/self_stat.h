/* self_stat.h -- get process stats from procfs on Linux
   v1.0
   Author: Vladimir Dinev
   vld.dinev@gmail.com
   2020-09-15
*/
#ifndef SELF_STAT_H
#define SELF_STAT_H

#ifndef __linux__
#error "/proc/self is on Linux and you are not"
#endif

#include <stdio.h>
#include <stddef.h>

typedef struct sst_proc_stat {
	int pid;
	char comm[16];
	char state;
	int ppid;
	int pgrp;
	int session;
	int tty_nr;
	int tpgid;
	unsigned int flags;
	unsigned long int minflt;
	unsigned long int cminflt;
	unsigned long int majflt;
	unsigned long int cmajflt;
	unsigned long int utime;
	unsigned long int stime;
	long int cutime;
	long int cstime;
	long int priority;
	long int nice;
	long int num_threads;
	long int itrealvalue;
	unsigned long long int starttime;
	unsigned long int vsize;
	long int rss;
	unsigned long int rsslim;
	unsigned long int startcode;
	unsigned long int endcode;
	unsigned long int startstack;
	unsigned long int kstkesp;
	unsigned long int kstkeip;
	unsigned long int signal;
	unsigned long int blocked;
	unsigned long int sigignore;
	unsigned long int sigcatch;
	unsigned long int wchan;
	unsigned long int nswap;
	unsigned long int cnswap;
	int exit_signal;
	int processor;
	unsigned int rt_priority;
	unsigned int policy;
	unsigned long long int delayacct_blkio_ticks;
	unsigned long int guest_time;
	long int cguest_time;
	unsigned long int start_data;
	unsigned long int end_data;
	unsigned long int start_brk;
	unsigned long int arg_start;
	unsigned long int arg_end;
	unsigned long int env_start;
	unsigned long int env_end;
	int exit_code;
} sst_proc_stat;

typedef struct sst_proc_statm {
	size_t vmsize;
	size_t resident;
	size_t shared;
	size_t text;
	size_t lib;
	size_t data;
	size_t dirty;
} sst_proc_statm;

void * sst_read_self_stat(sst_proc_stat * pstat);
/*
Reads /proc/self/stat into pstat, returns pstat on success, NULL on error.
*/

void sst_print_self_stat(const sst_proc_stat * pstat, FILE * where);
/*
Prints pstat to where.
*/

void * sst_read_self_statm(sst_proc_statm * pstatm);
/*
Reads /proc/self/statm into pstatm, returns pstatm on success, NULL on error.
*/

void sst_print_self_statm(const sst_proc_statm * pstatm, FILE * where);
/*
Prints pstatm to where.
*/

typedef void (*sst_per_line)(char * line, void * context);
void * sst_read_self_status(sst_per_line parse, void * context);
/*
Reads /proc/self/status and calls parse(line, context) for each line.
Returns parse on success, NULL on error. Note: fgets() is used for reading, so
the '\n' is a part of the line.
*/

void * sst_read_self_smaps(sst_per_line parse, void * context);
/*
Same as sst_read_self_status() but for /proc/self/smaps.
*/
#endif
