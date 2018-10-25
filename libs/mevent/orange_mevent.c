#include "orange_mevent.h"
#include "../orange/orange_log.h"
#include "orange_mevent_version.h"

ORANGE_VERSION_GENERATE(orange_mevent, 1, 1, 1, ORANGE_VERSION_TYPE_ALPHA);

static int __orange_mevent_module_init(void)
{

	snprintf(orange_mevent_description, 127, "Orange MEvent Module " ORANGE_VERSION_FORMAT "-%s #%u: %s", ORANGE_VERSION_QUAD(orange_mevent_version),
			 orange_version_type(orange_mevent_version_type), orange_mevent_build_num, orange_mevent_build_date);

	orange_log(ORANGE_LOG_INFO, "%s\n", orange_mevent_description);

	return 0;
}

static void __orange_mevent_module_fini(void)
{
	orange_log(ORANGE_LOG_INFO, "Orange MEvent Module unloaded.\n");

	return;
}

static int orange_mevent_modevent(orange_module_t mod, int type, void* data)
{
	int ret = 0;

	switch (type) {
		case ORANGE_MOD_LOAD:
			ret = __orange_mevent_module_init();
			break;
		case ORANGE_MOD_UNLOAD:
			__orange_mevent_module_fini();
			break;
		default:
			return (EOPNOTSUPP);
	}
	return ret;
}

static orange_moduledata_t orange_mevent_mod = {"orange_mevent", orange_mevent_modevent, 0};

ORANGE_DECLARE_MODULE(orange_mevent, orange_mevent_mod, ORANGE_SI_SUB_PSEUDO, ORANGE_SI_ORDER_ANY);

ORANGE_MODULE_VERSION(orange_mevent, 1);
ORANGE_DECLARE_MODULE_EXTENSION(orange_mevent);
