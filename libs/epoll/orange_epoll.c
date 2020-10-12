#include "orange_epoll.h"
#include "../orange/orange_log.h"
#include "../orange/orange_mutex.h"
#include "../orange/orange_queue.h"
#include "../orange/orange_thread.h"
#include "../orange/orange_tree.h"
#include "orange_epoll_version.h"

ORANGE_VERSION_GENERATE(orange_epoll, 1, 1, 1, ORANGE_VERSION_TYPE_ALPHA);

static int __orange_epoll_module_init(void)
{

	snprintf(orange_epoll_description, 127, "Orange Event Module " ORANGE_VERSION_FORMAT "-%s #%u: %s", ORANGE_VERSION_QUAD(orange_epoll_version),
			 orange_version_type(orange_epoll_version_type), orange_epoll_build_num, orange_epoll_build_date);

	orange_log(ORANGE_LOG_INFO, "%s\n", orange_epoll_description);

	return 0;
}

static void __orange_epoll_module_fini(void)
{
	orange_log(ORANGE_LOG_INFO, "Orange Event Module unloaded.\n");

	return;
}

static int orange_epoll_modepoll(orange_module_t mod, int type, void* data)
{
	int ret = 0;

	switch (type) {
		case ORANGE_MOD_LOAD:
			ret = __orange_epoll_module_init();
			break;
		case ORANGE_MOD_UNLOAD:
			__orange_epoll_module_fini();
			break;
		default:
			return (EOPNOTSUPP);
	}
	return ret;
}

static orange_moduledata_t orange_epoll_mod = {"orange_epoll", orange_epoll_modepoll, 0};

ORANGE_DECLARE_MODULE(orange_epoll, orange_epoll_mod, ORANGE_SI_SUB_PSEUDO, ORANGE_SI_ORDER_ANY);

ORANGE_MODULE_VERSION(orange_epoll, 1);
ORANGE_DECLARE_MODULE_EXTENSION(orange_epoll);
