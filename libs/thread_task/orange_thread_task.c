#include "orange_thread_task.h"
#include "../orange/orange_log.h"
#include "../orange/orange_mutex.h"
#include "../orange/orange_queue.h"
#include "../orange/orange_thread.h"
#include "../orange/orange_tree.h"
#include "orange_thread_task_version.h"

ORANGE_VERSION_GENERATE(orange_thread_task, 1, 1, 1, ORANGE_VERSION_TYPE_ALPHA);

typedef struct orange_thread_task_entry {
    char                                    task_name[ORANGE_THREAD_TASK_NAME_LEN_MAX];
    pthread_cond_t                          thread_cond;
	pthread_mutex_t                         thread_evmutex;
	pthread_condattr_t                      thread_cattr;
	uint32                                  event_flag;
    RB_ENTRY(orange_thread_task_entry)      entry;
} orange_thread_task_entry_t; 

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

int orange_thread_task_create(struct orange_thread_task_parameters *task_params, void* init_params, void* task_arg)
{
    int ret;
    pthread_t thread_id;
    pthread_attr_t thread_attr;  
    struct sched_param thead_sched;


    if (NULL == task_params || NULL == task_params->task_handler) {
        ret = -1;
        goto exit;
    }

    if (task_params->task_init) {
        task_params->task_init(init_params);
    }

    thread_sched.sched_priority = task_params->priority > 99 ? 99 : task_params->priority;
    thread_sched.sched_priority = thread_sched.sched_priority < 1 ? 1 : thread_sched.sched_priority;

    pthread_attr_init(&thread_attr);
    pthread_attr_setschedpolicy(&thread_attr,  SCHED_RR);
    pthread_attr_setinheritsched(&thread_attr, PTHREAD_EXPLICIT_SCHED);
    pthread_attr_setstacksize(&thread_attr, task_params->stack_size <= 65536 ? 65536 : task_params->stack_size);
    pthread_attr_setschedparam (&thread_attr,  &thread_sched);

    ret = pthread_create(&thread_id, &thread_attr, task_params->task_handler, task_arg);
    if (0 != ret) {
        pthread_attr_destroy(&thread_attr);
    }

exit:
    return ret;
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
