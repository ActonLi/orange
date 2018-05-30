#include "orange_syslog.h"
#include "../orange.h"
#include "../orange_log.h"

static int  system_log_level = ORANGE_LOG_INFO;
static char system_log_ident[ORANGE_LOG_IDENT_MAX];
static char system_log_buff[ORANGE_LOG_LINE_MAX];
static int  system_log_option   = -1;
static int  system_log_facility = -1;

static void orange_systemlog_open(void)
{
	openlog(system_log_buff, system_log_option, system_log_facility);
}

static void orange_systemlog_close(void)
{
	closelog();
}

int orange_systemlog_level_set(int level)
{
	if (level >= LOG_EMERG && level <= LOG_DEBUG) {
		system_log_level = level;
		return system_log_level;
	}
	return -1;
}

int orange_systemlog_level_get(void)
{
	return system_log_level;
}

int orange_systemlog_init(const char* ident, int option, int facility)
{
	memset(system_log_buff, 0, ORANGE_LOG_LINE_MAX);
	strncpy(system_log_ident, ident, ORANGE_LOG_IDENT_MAX);
	system_log_option   = option;
	system_log_facility = facility;
	orange_systemlog_level_set(ORANGE_LOG_INFO);
	orange_systemlog_open();
	return 0;
}

void orange_systemlog_fini(void)
{
	orange_systemlog_close();
	return;
}

int orange_systemlog_reinit()
{
	orange_systemlog_close();
	orange_systemlog_open();
	return 0;
}

void orange_systemlog_print(int level, char* buffer, int len)
{
	if (system_log_level >= level) {
		syslog(level | system_log_facility, buffer, len);
	}
}
