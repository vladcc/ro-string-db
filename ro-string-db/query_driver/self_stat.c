#include "self_stat.h"

#define pself(str) "/proc/self/" str

static const char fmt_str_stat[] = "%d %s %c %d %d %d %d %d %u %lu "
	"%lu %lu %lu %lu %lu %ld %ld %ld %ld %ld %ld %llu %lu %ld %lu %lu "
	"%lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %d %d %u %u %llu %lu "
	"%ld %lu %lu %lu %lu %lu %lu %lu %d";

void * sst_read_self_stat(sst_proc_stat * pstat)
{
	const int num_stats = 52;
	void * ret = NULL;
	FILE * fself = fopen(pself("stat"), "r");
	
	if (fself)
	{
		int stats = fscanf(fself, fmt_str_stat, &(pstat->pid),
			pstat->comm, &(pstat->state), &(pstat->ppid), &(pstat->pgrp),
			&(pstat->session), &(pstat->tty_nr), &(pstat->tpgid),
			&(pstat->flags), &(pstat->minflt), &(pstat->cminflt),
			&(pstat->majflt), &(pstat->cmajflt), &(pstat->utime),
			&(pstat->stime), &(pstat->cutime), &(pstat->cstime),
			&(pstat->priority), &(pstat->nice), &(pstat->num_threads),
			&(pstat->itrealvalue), &(pstat->starttime), &(pstat->vsize),
			&(pstat->rss), &(pstat->rsslim), &(pstat->startcode),
			&(pstat->endcode), &(pstat->startstack), &(pstat->kstkesp),
			&(pstat->kstkeip), &(pstat->signal), &(pstat->blocked),
			&(pstat->sigignore), &(pstat->sigcatch), &(pstat->wchan),
			&(pstat->nswap), &(pstat->cnswap), &(pstat->exit_signal),
			&(pstat->processor), &(pstat->rt_priority), &(pstat->policy),
			&(pstat->delayacct_blkio_ticks), &(pstat->guest_time),
			&(pstat->cguest_time), &(pstat->start_data), &(pstat->end_data),
			&(pstat->start_brk), &(pstat->arg_start), &(pstat->arg_end),
			&(pstat->env_start), &(pstat->env_end), &(pstat->exit_code)
		);
		
		fclose(fself);
		if (num_stats == stats)
			ret = pstat;
	}
	
	return ret;
}

void sst_print_self_stat(const sst_proc_stat * pstat, FILE * where)
{
	fprintf(where, fmt_str_stat, pstat->pid, pstat->comm,
		pstat->state, pstat->ppid, pstat->pgrp,
		pstat->session, pstat->tty_nr, pstat->tpgid,
		pstat->flags, pstat->minflt, pstat->cminflt,
		pstat->majflt, pstat->cmajflt, pstat->utime,
		pstat->stime, pstat->cutime, pstat->cstime,
		pstat->priority, pstat->nice, pstat->num_threads,
		pstat->itrealvalue, pstat->starttime, pstat->vsize,
		pstat->rss, pstat->rsslim, pstat->startcode,
		pstat->endcode, pstat->startstack, pstat->kstkesp,
		pstat->kstkeip, pstat->signal, pstat->blocked,
		pstat->sigignore, pstat->sigcatch, pstat->wchan,
		pstat->nswap, pstat->cnswap, pstat->exit_signal,
		pstat->processor, pstat->rt_priority, pstat->policy,
		pstat->delayacct_blkio_ticks, pstat->guest_time,
		pstat->cguest_time, pstat->start_data, pstat->end_data,
		pstat->start_brk, pstat->arg_start, pstat->arg_end,
		pstat->env_start, pstat->env_end, pstat->exit_code
	);
	fputc('\n', where);
}

static const char fmt_str_statm[] = "%zu %zu %zu %zu %zu %zu %zu";

void * sst_read_self_statm(sst_proc_statm * pstatm)
{
	const int num_stats = 7;
	void * ret = NULL;
	FILE * fself = fopen(pself("statm"), "r");
	
	if (fself)
	{
		int stats = fscanf(fself, fmt_str_statm, &(pstatm->vmsize),
			&(pstatm->resident), &(pstatm->shared), &(pstatm->text),
			&(pstatm->lib), &(pstatm->data), &(pstatm->dirty)
		);
		fclose(fself);
		
		if (num_stats == stats)
			ret = pstatm;
	}
	
	return ret;
}

void sst_print_self_statm(const sst_proc_statm * pstatm, FILE * where)
{
	fprintf(where, fmt_str_statm, pstatm->vmsize, pstatm->resident,
		pstatm->shared, pstatm->text, pstatm->lib, pstatm->data, pstatm->dirty
	);
	fputc('\n', where);
}

static void * for_each_line(const char * fname, sst_per_line fn, void * context)
{
#define BUFF_SZ 1024
	static char buff[BUFF_SZ];
	
	void * ret = NULL;
	
	FILE * fself = fopen(fname, "r");
	if (fself)
	{
		while (fgets(buff, BUFF_SZ, fself))
			fn(buff, context);
		
		if (!ferror(fself))
			ret = (void *)fn;
			
		fclose(fself);
	}
	
	return ret;
#undef BUFF_SZ
}

void * sst_read_self_status(sst_per_line parse, void * context)
{
	return for_each_line(pself("status"), parse, context);
}

void * sst_read_self_smaps(sst_per_line parse, void * context)
{
	return for_each_line(pself("smaps"), parse, context);
}
