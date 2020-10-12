#include "orange_hashtable.h"
#include "../orange/orange_log.h"
#include "../orange/orange_mutex.h"
#include "../orange/orange_queue.h"
#include "../orange/orange_thread.h"
#include "../orange/orange_tree.h"
#include "orange_hashtable_version.h"

ORANGE_VERSION_GENERATE(orange_hashtable, 1, 1, 1, ORANGE_VERSION_TYPE_ALPHA);

static int __orange_hashtable_module_init(void)
{

	snprintf(orange_hashtable_description, 127, "Orange Event Module " ORANGE_VERSION_FORMAT "-%s #%u: %s", ORANGE_VERSION_QUAD(orange_hashtable_version),
			 orange_version_type(orange_hashtable_version_type), orange_hashtable_build_num, orange_hashtable_build_date);

	orange_log(ORANGE_LOG_INFO, "%s\n", orange_hashtable_description);

	return 0;
}

static void __orange_hashtable_module_fini(void)
{
	orange_log(ORANGE_LOG_INFO, "Orange Event Module unloaded.\n");

	return;
}

static int orange_hashtable_modhashtable(orange_module_t mod, int type, void* data)
{
	int ret = 0;

	switch (type) {
		case ORANGE_MOD_LOAD:
			ret = __orange_hashtable_module_init();
			break;
		case ORANGE_MOD_UNLOAD:
			__orange_hashtable_module_fini();
			break;
		default:
			return (EOPNOTSUPP);
	}
	return ret;
}

static orange_moduledata_t orange_hashtable_mod = {"orange_hashtable", orange_hashtable_modhashtable, 0};

ORANGE_DECLARE_MODULE(orange_hashtable, orange_hashtable_mod, ORANGE_SI_SUB_PSEUDO, ORANGE_SI_ORDER_ANY);

ORANGE_MODULE_VERSION(orange_hashtable, 1);
ORANGE_DECLARE_MODULE_EXTENSION(orange_hashtable);
