#include "watchdog.h"
#include "orange.h"
#include "orange_daemon.h"
#include "orange_log.h"
#include "orange_module.h"
#include "orange_options.h"
#include "orange_utils.h"

#define DEFAULT_WATCHDOG_CONFIG "wtd.conf"

typedef struct watchdog_options {
	int   daemon;
	char* config_file;
} watchdog_options_t;

struct watchdog_options opt;

static void __watchdog_help(const char* opt, const char* arg, int l)
{
	printf("Usage: %s [-Dh] [-c config-file] ...\n", "./watchdog");
	exit(1);
}

static void __watchdog_parse_options(int argc, char** argv)
{
	orange_parse_options(&argc, &argv, OPTIONS(FLAG('D', "Daemon", opt.daemon, 1), PARAMETER('c', "configure-file", opt.config_file),
											   FLAG_CALLBACK('h', "help", __watchdog_help)));
}

int main(int argc, char** argv)
{
	opt.config_file								 = DEFAULT_WATCHDOG_CONFIG;
	struct orange_module_session* module_session = NULL;

	__watchdog_parse_options(argc, argv);

	module_session = orange_module_open("/home/orange/git/orange/apps/watchdog");
	if (module_session == NULL) {
		return -1;
	}

	orange_module_load_all(module_session);

	if (opt.daemon) {
		orange_daemon_create(orange_get_short_proc_name(argv[0]));
	}

	while (1)
		;

	return 0;
}
