#include "orange_filelog.h"
#include "../orange.h"
#include "../orange_log.h"

static int  file_log_level = ORANGE_LOG_INFO;
static char file_log_ident[ORANGE_LOG_IDENT_MAX];
static char file_log_buff[ORANGE_LOG_LINE_MAX];
static char file_log_filename[PATH_MAX];
static int  file_log_fd = -1;

static int orange_filelog_open(void)
{
	file_log_fd = open(file_log_filename, O_CREAT | O_APPEND | O_WRONLY, 00644);
	return file_log_fd;
}

static void orange_filelog_close(void)
{
	if (file_log_fd > 0)
		close(file_log_fd);
}

static void orange_filelog_raw_printf(const char* buffer, int len)
{
	int unused __attribute__((unused));

	if (file_log_fd > 0) {
		unused = write(file_log_fd, buffer, len);
	}
}

int orange_filelog_level_set(int level)
{
	if (level >= LOG_EMERG && level <= LOG_DEBUG) {
		file_log_level = level;
		return file_log_level;
	}
	return -1;
}

int orange_filelog_level_get(void)
{
	return file_log_level;
}

int orange_filelog_init(const char* ident, const char* logfile)
{
	memset(file_log_buff, 0, ORANGE_LOG_LINE_MAX);
	strncpy(file_log_ident, ident, ORANGE_LOG_IDENT_MAX);
	strncpy(file_log_filename, logfile, PATH_MAX);
	orange_filelog_level_set(ORANGE_LOG_INFO);
	if (orange_filelog_open() == -1)
		return -1;
	return 0;
}

void orange_filelog_fini(void)
{
	orange_filelog_close();
	return;
}

int orange_filelog_reinit()
{
	orange_filelog_close();
	if (orange_filelog_open() == -1)
		return -1;
	return 0;
}

void orange_filelog_print(int level, char* buffer, int len)
{
	char time[ORANGE_LOG_TIME_BUFF_SIZE];

	if (file_log_level >= level) {
		orange_log_get_time(time);
		snprintf(file_log_buff, 2047, "%s %s: %s\n", time, file_log_ident, buffer);
		orange_filelog_raw_printf(file_log_buff, strlen(file_log_buff));
	}
}
