#include "upfw.h"
#include "daemon.h"
#include "events.h"
#include "orangelog.h"
#include "osa_thr.h"
#include "osa_tick.h"
#include "thread_main.h"
#include "var.h"
#include "watchdog_client.h"

#if 0
#define DEBUGP printf
#else
#define DEBUGP(format, args...)
#endif

#if 0
#define DEBUGP_TEST printf
#else
#define DEBUGP_TEST(format, args...)
#endif

struct config_info config;
int				   check_status = 0;
static char		   daemon_name[DAEMON_NAME_SIZE];
U64				   last_check_timestamp;

static void __upfw_usage(void)
{
	printf("Usage: upfw [options]\n \
            To invoke the menu, type:\n \
            upfw -menu\n \
            The options are:\n \
            -c     <STRING>        firmwares infomation\n \
            -u     <STRING>        server url\n \
            -d     <STRING>        update directory\n \
            -k     <STRING>        rsa public key\n \
            -D     <TOGGLE>        run as daemon\n \
            -h     <TOGGLE>        help\n");
	return;
}

static int __upfw_parse_option_value(char option, char* value)
{
	switch (option) {
		case 'D':
			config.daemon = 1;
			break;
		case 'c':
			if (value) {
				if (config.firmware_info) {
					free(config.firmware_info);
				}
				config.firmware_info = strdup(value);
			}
			break;
		case 'u':
			if (value) {
				if (config.update_url) {
					free(config.update_url);
				}
				config.update_url = strdup(value);
			}
			break;
		case 'd':
			if (value) {
				if (config.update_dir) {
					free(config.update_dir);
				}
				config.update_dir = strdup(value);
			}
			break;
		case 'k':
			if (value) {
				if (config.public_key) {
					free(config.public_key);
				}
				config.public_key = strdup(value);
			}
			break;
		case 'h':
			__upfw_usage();
			break;
		default:
			break;
	}

	return 0;
}

static int __upfw_parse_daemon_name(char* p_name)
{
	int   ret = -1;
	char* p   = p_name;
	char* end = NULL;

	if (p_name == NULL) {
		goto exit;
	}

	p   = p_name + strlen(p_name) - 1;
	end = p;

	while (1) {
		if (p == p_name || *p == '/') {
			break;
		}
		p--;
	}

	if (*p == '/') {
		p++;
	}

	memcpy(daemon_name, p, end - p + 1);

	ret = 0;
exit:
	return ret;
}

static int __upfw_parse_argv(int* argc, char*** argv)
{
	int ret		  = -1;
	int para_nums = *argc;
	int i;
	int __attribute__((unused)) len;
	char* para;
	int   value_index;
	char  option;
	char* value;

	__upfw_parse_daemon_name((*argv)[0]);

	if (para_nums < 2) {
		ret = 0;
		goto exit;
	}

	for (i = 1; i < para_nums; i++) {
		para = (*argv)[i];
		if (para == NULL) {
			break;
		}
		len = strlen(para);
		if (*para == '-') {
			option = *(para + 1);
			if (option == 'D') {
				__upfw_parse_option_value(option, NULL);
			} else if (option == 'h' || option == 'H') {
				__upfw_usage();
				exit(0);
			} else {
				value_index = i + 1;
				if (value_index >= para_nums) {
					break;
				}
				value = (*argv)[value_index];
				__upfw_parse_option_value(option, value);
			}
		}
	}

	ret = 0;
exit:
	return ret;
}

static char log[WATCHDOG_LOG_LINE_MAX];

int upfw_send_log(char* fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	vsnprintf(log, WATCHDOG_LOG_LINE_MAX, fmt, args);
	va_end(args);

	return watchdog_client_send_msg_by_method(WATCHDOG_TASK_LOG, SOCK_UPFW_ID, SOCK_WATCHDOG_ID, "log", log);
}

int main(int argc, char* argv[])
{
	U32 counter = 0;

	last_check_timestamp = time(NULL) - CHECK_TIMEOUT;

	config.firmware_info = strdup(DEFALT_JSON_FILE);
	config.update_url	= strdup(UPDATE_URL);
	config.update_dir	= strdup(UPDATE_DIR);
	config.public_key	= strdup(RSA_PUBKEY);

	__upfw_parse_argv(&argc, &argv);

	if (-1 == access(config.update_dir, R_OK | W_OK | X_OK)) {
		mkdir(config.update_dir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	}

	if (-1 == access(DEFALT_JSON_FILE_DIR, R_OK | W_OK | X_OK)) {
		mkdir(DEFALT_JSON_FILE_DIR, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	}

	if (config.daemon) {
		daemon_create(daemon_name);
	} else {
		if (daemon_already_running(daemon_name)) {
			printf("%s is already running.\n", daemon_name);
			exit(1);
		}
	}
	orangelog_init(daemon_name, -1, 1, -1);

	watchdog_client_register(SOCK_UPFW_ID, SOCK_WATCHDOG_ID, CLIENT_TYPE_REPORT, 10, "upfw", NULL, NULL);
	orangelog_log_info("Watchdog register successful.\n");

	event_init();
	orangelog_log_info("Eevent module load successful.\n");
	OSA_InitTick(800);
	thread_main_init();
	orangelog_log_info("Thread module load successful.\n");
	while (1) {
		OSA_Sleep(10000);
		counter++;
	}
	event_uninit();

	return 0;
}
