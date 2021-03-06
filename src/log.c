#include "burp.h"
#include "conf.h"
#include "log.h"

static const char *prog="unknown";

static FILE *logfp=NULL;
static int do_syslog=0;

void init_log(char *progname)
{
	if((prog=strrchr(progname, '/'))) prog++;
	else prog=progname;
}

static char *gettm(void)
{
        time_t t=0;
        const struct tm *ctm=NULL;
        static char tmbuf[32]="";

        time(&t);
        ctm=localtime(&t);
	// Windows does not like the %T strftime format option - you get
	// complaints under gdb.
        strftime(tmbuf, sizeof(tmbuf), "%Y-%m-%d %H:%M:%S", ctm);
	return tmbuf;
}

void logp(const char *fmt, ...)
{
	int pid;
	char buf[512]="";
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(buf, sizeof(buf), fmt, ap);
	pid=(int)getpid();
	fprintf(logfp?logfp:stdout, "%s: %s[%d] %s", gettm(), prog, pid, buf);
	if(do_syslog)
		syslog(LOG_INFO, "%s: %s[%d] %s", gettm(), prog, pid, buf);
	va_end(ap);
}

/* for the counters */
void logc(const char *fmt, ...)
{
	char buf[512]="";
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(buf, sizeof(buf), fmt, ap);
	fprintf(logfp?logfp:stdout, "%s", buf);
	va_end(ap);
}

const char *progname(void)
{
	return prog;
}

int set_logfp(FILE *fp, struct config *conf)
{
	if(logfp) fclose(logfp);
	logfp=fp;
#ifndef HAVE_WIN32
	if(logfp) setlinebuf(logfp);
#endif
	do_syslog=conf->syslog;
	return 0;
}

FILE *get_logfp(void)
{
	return logfp;
}

int open_logfile(const char *logfile, struct config *conf)
{
	FILE *fp=NULL;
	set_logfp(NULL, conf); // Close the old log, if it is open.
	if(!logfile || !*logfile) return 0;
	if(!(fp=fopen(logfile, "ab")))
	{
		logp("error opening logfile %s.\n", logfile);
		return 1;
	}
	set_logfp(fp, conf);
	return 0;
}
