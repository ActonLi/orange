#include "orange_watchdog.h"
#include "../orange/orange_queue.h"
#include "../orange/orange_spinlock.h"
#include "../orange/orange_tree.h"
#include "../timer/orange_timer.h"
#include "orange_watchdog_version.h"

ORANGE_VERSION_GENERATE(orange_watchdog, 1, 1, 1, ORANGE_VERSION_TYPE_ALPHA);

static int __orange_timeout_func(int id, void* data, int data_len)
{
	orange_log(ORANGE_LOG_INFO, "%s:%d.\n", __func__, __LINE__);
	return 0;
}

static int orange_watchdog_init(void)
{
	snprintf(orange_watchdog_description, 127, "Orange Watchdog Module " ORANGE_VERSION_FORMAT "-%s #%u: %s", ORANGE_VERSION_QUAD(orange_watchdog_version),
			 orange_version_type(orange_watchdog_version_type), orange_watchdog_build_num, orange_watchdog_build_date);

	orange_log(ORANGE_LOG_INFO, "%s\n", orange_watchdog_description);
	orange_timer_set(100, ORANGE_TIMER_CONTINUED, __orange_timeout_func, NULL);

	return 0;
}

static void orange_watchdog_fini(void)
{
	return;
}

static int orange_watchdog_modevent(orange_module_t mod, int type, void* data)
{
	int ret = 0;

	switch (type) {
		case ORANGE_MOD_LOAD:
			ret = orange_watchdog_init();
			break;
		case ORANGE_MOD_UNLOAD:
			orange_watchdog_fini();
			break;
		default:
			return (EOPNOTSUPP);
	}
	return ret;
}

static orange_moduledata_t orange_watchdog_mod = {"orange_watchdog", orange_watchdog_modevent, 0};

ORANGE_DECLARE_MODULE(orange_watchdog, orange_watchdog_mod, ORANGE_SI_SUB_PSEUDO, ORANGE_SI_ORDER_ANY);

ORANGE_MODULE_DEPEND(orange_watchdog, orange_timer, 1, 1, 1);
ORANGE_MODULE_VERSION(orange_watchdog, 1);
ORANGE_DECLARE_MODULE_EXTENSION(orange_watchdog);
