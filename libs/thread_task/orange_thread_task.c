#include "orange_thread_task.h"
#include "../orange/orange_log.h"
#include "../orange/orange_mutex.h"
#include "../orange/orange_queue.h"
#include "../orange/orange_thread.h"
#include "../orange/orange_tree.h"
#include "orange_thread_task_version.h"

ORANGE_VERSION_GENERATE(orange_thread_task, 1, 1, 1, ORANGE_VERSION_TYPE_ALPHA);

static int __orange_thread_task_module_init(void)
{
	snprintf(orange_thread_task_description, 127, "Orange thread task module " ORANGE_VERSION_FORMAT "-%s #%u: %s", ORANGE_VERSION_QUAD(orange_thread_task_version),
			 orange_version_type(orange_thread_task_version_type), orange_thread_task_build_num, orange_thread_task_build_date);

	orange_log(ORANGE_LOG_INFO, "%s\n", orange_thread_task_description);

	return 0;
}

static void __orange_thread_task_module_fini(void)
{
	orange_log(ORANGE_LOG_INFO, "Orange thread task module unloaded.\n");

	return;
}

static int orange_thread_task_modthread_task(orange_module_t mod, int type, void* data)
{
	int ret = 0;

	switch (type) {
		case ORANGE_MOD_LOAD:
			ret = __orange_thread_task_module_init();
			break;
		case ORANGE_MOD_UNLOAD:
			__orange_thread_task_module_fini();
			break;
		default:
			return (EOPNOTSUPP);
	}
	return ret;
}

static orange_moduledata_t orange_thread_task_mod = {"orange_thread_task", orange_thread_task_modthread_task, 0};

ORANGE_DECLARE_MODULE(orange_thread_task, orange_thread_task_mod, ORANGE_SI_SUB_PSEUDO, ORANGE_SI_ORDER_ANY);

ORANGE_MODULE_VERSION(orange_thread_task, 1);
ORANGE_DECLARE_MODULE_EXTENSION(orange_thread_task);
