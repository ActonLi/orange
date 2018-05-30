#include "orange_log.h"
#include "log/orange_consolelog.h"
#include "log/orange_filelog.h"
#include "log/orange_syslog.h"

static char log_ident[ORANGE_LOG_IDENT_MAX];
static int  log_level = ORANGE_LOG_INFO;

static int log_syslog	 = -1;
static int log_filelog	= -1;
static int log_consolelog = -1;

static int log_syslog_inited	 = -1;
static int log_filelog_inited	= -1;
static int log_consolelog_inited = -1;

static char log_buff[ORANGE_LOG_LINE_MAX];
static char log_filename[PATH_MAX];

int orange_log_level_set(int level)
{
	if (level >= ORANGE_LOG_EMERG && level <= ORANGE_LOG_DEBUG) {
		log_level = level;
		orange_systemlog_level_set(level);
		orange_consolelog_level_set(level);
		orange_filelog_level_set(level);
	}
	return -1;
}

int orange_log_level_get(void)
{
	return log_level;
}

int orange_log_open()
{
	if (log_syslog == 1) {
		if (log_syslog_inited != -1) {
			orange_systemlog_fini();
		}
		log_syslog_inited = orange_systemlog_init(log_ident, LOG_NDELAY, LOG_USER);
	}
	if (log_consolelog == 1) {
		if (log_consolelog_inited != -1) {
			orange_consolelog_fini();
		}
		log_consolelog_inited = orange_consolelog_init(log_ident);
	}
	if (log_filelog == 1) {
		if (log_filelog_inited != -1) {
			orange_filelog_fini();
		}
		sprintf(log_filename, "/var/log/%s.log", log_ident);
		log_filelog_inited = orange_filelog_init(log_ident, log_filename);
	}
	if (log_syslog_inited == -1 && log_consolelog_inited == -1 && log_filelog_inited == -1) {
		return -1;
	}
	orange_log_level_set(orange_log_level_get());
	return 0;
}

void orange_log_close(void)
{
	if (log_syslog == 1 && log_syslog_inited != -1) {
		orange_systemlog_reinit();
	}
	if (log_filelog == 1 && log_filelog_inited != -1) {
		orange_filelog_reinit();
	}
	if (log_consolelog == 1 && log_consolelog_inited != -1) {
		orange_consolelog_reinit();
	}
}

int orange_log_init(const char* ident, int syslog, int filelog, int consolelog)
{
	log_syslog	 = syslog;
	log_filelog	= filelog;
	log_consolelog = consolelog;
	strncpy(log_ident, ident, ORANGE_LOG_IDENT_MAX);
	return orange_log_open();
}

int orange_log_reinit()
{
	orange_log_close();
	return orange_log_open();
}

void orange_log_fini(void)
{
	if (log_syslog && log_syslog_inited != -1) {
		orange_systemlog_fini();
	}
	if (log_filelog && log_filelog_inited != -1) {
		orange_filelog_fini();
	}
	if (log_consolelog && log_consolelog_inited != -1) {
		orange_consolelog_fini();
	}
}

int orange_log_string(int level, char* buff, int len)
{
	if (len > 1 && buff[len - 1] == '\n') {
		buff[len - 1] = '\0';
	}

	if (log_syslog_inited != -1) {
		orange_systemlog_print(level, buff, len);
	}
	if (log_filelog_inited != -1) {
		orange_filelog_print(level, buff, len);
	}
	if (log_consolelog_inited != -1) {
		orange_consolelog_print(level, buff, len);
	}

	if (log_syslog_inited == -1 && log_filelog_inited == -1 && log_consolelog_inited == -1) {
		orange_consolelog_print(level, buff, len);
	}

	return len;
}

int orange_log(int level, const char* fmt, ...)
{
	int ret;

	va_list args;
	va_start(args, fmt);
	vsnprintf(log_buff, ORANGE_LOG_LINE_MAX, fmt, args);
	ret = orange_log_string(level, log_buff, strlen(log_buff));
	va_end(args);

	return ret;
}

int orange_log_debug(const char* fmt, ...)
{
	int ret;

	va_list args;
	va_start(args, fmt);
	vsnprintf(log_buff, ORANGE_LOG_LINE_MAX, fmt, args);
	ret = orange_log_string(ORANGE_LOG_DEBUG, log_buff, strlen(log_buff));
	va_end(args);

	return ret;
}

int orange_log_info(const char* fmt, ...)
{
	int ret;

	va_list args;
	va_start(args, fmt);
	vsnprintf(log_buff, ORANGE_LOG_LINE_MAX, fmt, args);
	ret = orange_log_string(ORANGE_LOG_INFO, log_buff, strlen(log_buff));
	va_end(args);

	return ret;
}

int orange_log_notice(const char* fmt, ...)
{
	int ret;

	va_list args;
	va_start(args, fmt);
	vsnprintf(log_buff, ORANGE_LOG_LINE_MAX, fmt, args);
	ret = orange_log_string(ORANGE_LOG_NOTICE, log_buff, strlen(log_buff));
	va_end(args);

	return ret;
}

int orange_log_warning(const char* fmt, ...)
{
	int ret;

	va_list args;
	va_start(args, fmt);
	vsnprintf(log_buff, ORANGE_LOG_LINE_MAX, fmt, args);
	ret = orange_log_string(ORANGE_LOG_WARNING, log_buff, strlen(log_buff));
	va_end(args);

	return ret;
}

int orange_log_alert(const char* fmt, ...)
{
	int ret;

	va_list args;
	va_start(args, fmt);
	vsnprintf(log_buff, ORANGE_LOG_LINE_MAX, fmt, args);
	ret = orange_log_string(ORANGE_LOG_ALERT, log_buff, strlen(log_buff));
	va_end(args);

	return ret;
}

int orange_log_error(const char* fmt, ...)
{
	int ret;

	va_list args;
	va_start(args, fmt);
	vsnprintf(log_buff, ORANGE_LOG_LINE_MAX, fmt, args);
	ret = orange_log_string(ORANGE_LOG_ERR, log_buff, strlen(log_buff));
	va_end(args);

	return ret;
}

int orange_log_crit(const char* fmt, ...)
{
	int ret;

	va_list args;
	va_start(args, fmt);
	vsnprintf(log_buff, ORANGE_LOG_LINE_MAX, fmt, args);
	ret = orange_log_string(ORANGE_LOG_CRIT, log_buff, strlen(log_buff));
	va_end(args);

	return ret;
}
