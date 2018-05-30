#include "orange_consolelog.h"
#include "../orange.h"
#include "../orange_log.h"

static int  console_log_level						= ORANGE_LOG_INFO;
static char console_log_ident[ORANGE_LOG_IDENT_MAX] = "orange";
static char console_log_buff[ORANGE_LOG_LINE_MAX];

int orange_consolelog_level_set(int level)
{
	if (level >= ORANGE_LOG_EMERG && level <= ORANGE_LOG_DEBUG) {
		console_log_level = level;
		return console_log_level;
	}
	return -1;
}

int orange_consolelog_level_get(void)
{
	return console_log_level;
}

int orange_consolelog_init(const char* ident)
{
	memset(console_log_buff, 0, ORANGE_LOG_LINE_MAX);
	strncpy(console_log_ident, ident, ORANGE_LOG_IDENT_MAX);
	orange_consolelog_level_set(ORANGE_LOG_INFO);
	return 0;
}

void orange_consolelog_fini(void)
{
	return;
}

int orange_consolelog_reinit()
{
	return 0;
}

void orange_consolelog_print(int level, char* buffer, int len)
{
	char time[ORANGE_LOG_TIME_BUFF_SIZE];

	if (console_log_level >= level) {
		orange_log_get_time(time);
		snprintf(console_log_buff, ORANGE_LOG_LINE_MAX - 1, "%s %s: %s\n", time, console_log_ident, buffer);
		printf("%s", console_log_buff);
	}
}
