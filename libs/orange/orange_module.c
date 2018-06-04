#include "orange_module.h"
#include "orange.h"
#include "orange_elf.h"
#include "orange_log.h"
#include "orange_queue.h"
#include "orange_spinlock.h"

#if 0
#define DEBUGP printf
#else
#define DEBUGP(format, args...)
#endif

#define ORANGE_MODULE_CONFIG_FILENAME "modules.conf"

#define ORANGE_MODULE_DEPEND_LINK_SIZE 16

typedef struct orange_module_depend_link {
	struct orange_module_entry*		  entrys[ORANGE_MODULE_DEPEND_LINK_SIZE];
	struct orange_module_depend_link* next;
} orange_module_depend_link_t;

typedef struct orange_module_entry {
	uint16_t						  use;
	void*							  handle;
	struct orange_module*			  module;
	struct orange_module_depend_link* depend_link;
	TAILQ_ENTRY(orange_module_entry) entry;
} orange_module_entry_t;

typedef struct orange_module_session {
	char			  path[PATH_MAX];
	int				  count;
	orange_spinlock_t lock;
	TAILQ_HEAD(orange_module_list, orange_module_entry) list;
} orange_module_session_t;

typedef struct orange_module_params {
	struct orange_module_session*	 module_session;
	char*							  module_name;
	struct orange_module_depend_link* module_depend_link;
} orange_module_params_t;

typedef struct orange_module_sysinit_params {
	struct orange_module_session* module_session;
	char*						  module_name;
	void*						  handle;
	uint8_t						  init;
} orange_module_sys_init_params_t;

static int __orange_module_load_unlock(struct orange_module_session* module_session, char* module_name, int depended,
									   struct orange_module_depend_link* module_depend_link);

static int __orange_module_depend_link_inc(struct orange_module_depend_link* module_depend_link, struct orange_module_entry* module_entry)
{
	int								  ret = ENOMEM;
	int								  i;
	struct orange_module_depend_link* next_depend_link;
	struct orange_module_depend_link* new_depend_link;

	DEBUGP("%s begin: depend_link: %p, module_name: %s\n", __func__, module_depend_link, module_entry->module->moduledata->name);

	next_depend_link = module_depend_link;

	while (next_depend_link != NULL) {
		for (i = 0; i < ORANGE_MODULE_DEPEND_LINK_SIZE; i++) {
			if (next_depend_link->entrys[i] == NULL) {
				next_depend_link->entrys[i] = module_entry;
				ret							= 0;
				goto exit;
			}
		}
		if (next_depend_link->next != NULL) {
			next_depend_link = next_depend_link->next;
		} else {
			new_depend_link		   = orange_zalloc(sizeof(struct orange_module_depend_link));
			next_depend_link->next = new_depend_link;
			next_depend_link	   = new_depend_link;
		}
	}
exit:
	DEBUGP("%s: finished, ret = %d\n", __func__, ret);
	return ret;
}

static void __orange_module_depend_link_put(struct orange_module_depend_link* module_depend_link)
{
	int								  i;
	struct orange_module_depend_link* next_depend_link = NULL;
	struct orange_module_depend_link* prev_depend_link;
	struct orange_module_entry*		  module_entry;

	DEBUGP("%s begin: depend_link: %p\n", __func__, module_depend_link);

	prev_depend_link = module_depend_link;
	next_depend_link = module_depend_link;

	while (prev_depend_link != NULL) {
		next_depend_link = prev_depend_link->next;

		prev_depend_link->next = NULL;

		/* dec use count */
		for (i = 0; i < ORANGE_MODULE_DEPEND_LINK_SIZE; i++) {
			if (prev_depend_link->entrys[i] != NULL) {
				module_entry = prev_depend_link->entrys[i];
				module_entry->use--;

				DEBUGP("%s: dec depended module use: %s/%d\n", __func__, module_entry->module->moduledata->name, module_entry->use);
			}
		}

		/* free dynamic link entry */
		orange_free(prev_depend_link);

		prev_depend_link = next_depend_link;
	}

	DEBUGP("%s: finished\n", __func__);
	return;
}

static struct orange_module_entry* __orange_module_get(struct orange_module_session* module_session, char* module_name)
{
	struct orange_module_entry* module_entry;

	TAILQ_FOREACH(module_entry, &module_session->list, entry)
	{
		if ((strlen(module_name) == strlen(module_entry->module->moduledata->mod_name)) &&
			strncmp(module_entry->module->moduledata->mod_name, module_name, strlen(module_name)) == 0) {
			break;
		}
	}

	return module_entry;
}

static int __orange_module_exist(struct orange_module_session* module_session, char* module_name)
{
	int							exist = 0;
	struct orange_module_entry* module_entry;

	TAILQ_FOREACH(module_entry, &module_session->list, entry)
	{
		if ((strlen(module_name) == strlen(module_entry->module->moduledata->mod_name)) &&
			strncmp(module_entry->module->moduledata->mod_name, module_name, strlen(module_name)) == 0) {
			exist = 1;
		}
	}

	return exist;
}

/* sysinit */
static int __orange_module_sysinit_all(char* name, uint64_t value, void* data)
{
	int									 ret = 0;
	struct orange_module_sysinit_params* params;
	char								 base_string[PATH_MAX];

	/* Name endsWith "object" */
	if (strlen(name) > strlen("object") && memcmp((name + strlen(name) - strlen("object")), "object", strlen("object")) == 0) {
		goto exit;
	}

	params = (struct orange_module_sysinit_params*) data;
	if (params->init) {
		snprintf(base_string, PATH_MAX, "sys_init_");
	} else {
		snprintf(base_string, PATH_MAX, "sys_uninit_");
	}

	if (strlen(name) > strlen(base_string)) {
		if (strncmp(base_string, name, strlen(base_string)) == 0) {
			orange_module_sysinit_get_func_t func;
			struct orange_sysinit*			 sysinit;

			DEBUGP("%s: dlsym: %s\n", __func__, name);
			func = (orange_module_sysinit_get_func_t) dlsym(params->handle, name);
			if (func == NULL) {
				orange_log(ORANGE_LOG_ERR, "loading sysinit '%s' faild: %s\n", name, dlerror());
				ret = EINVAL;
				goto exit;
			}

			sysinit = func();
			sysinit->func(sysinit->udata);

			DEBUGP("%s: finished, ret = %d\n", __func__, ret);
		}
	}

exit:
	return ret;
}

static int __orange_module_sysinit(struct orange_module_session* module_session, char* module_name, void* handle, uint8_t init)
{
	int									ret;
	char								path[PATH_MAX];
	struct orange_elf_file*				elf_file = NULL;
	struct orange_module_sysinit_params params;

	DEBUGP("%s: path: %s, module_name: %s\n", __func__, module_session->path, module_name);

	snprintf(path, PATH_MAX, "%s/%s.so", module_session->path, module_name);

	elf_file = orange_elf_open(path);
	if (elf_file == NULL) {
		ret = ENOENT;
		orange_log(ORANGE_LOG_ERR, "load module '%s' faild: %d\n", module_name, ret);
		goto exit;
	}

	params.module_session = module_session;
	params.module_name	= module_name;
	params.init			  = init;
	params.handle		  = handle;

	ret = orange_elf_symbol(elf_file, __orange_module_sysinit_all, &params);

exit:
	if (elf_file) {
		orange_elf_close(elf_file);
	}

	return ret;
}

/* load and run */
static int ____orange_module_load(struct orange_module_session* module_session, char* module_name, struct orange_module_entry* module_entry)
{
	int							  ret;
	void*						  handle = NULL;
	char						  path[PATH_MAX];
	char						  func_name[64];
	orange_module_info_get_func_t func;

	DEBUGP("%s: path: %s, module_name: %s\n", __func__, module_session->path, module_name);
	snprintf(path, PATH_MAX, "%s/%s.so", module_session->path, module_name);

	DEBUGP("%s: dlopen: %s\n", __func__, path);
	handle = dlopen(path, RTLD_NOW | RTLD_GLOBAL);
	if (handle == NULL) {
		orange_log(ORANGE_LOG_ERR, "loading module '%s' faild: %s\n", module_name, dlerror());
		ret = EINVAL;
		goto exit;
	}

	snprintf(func_name, 64, "%s_module", module_name);

	DEBUGP("%s: dlsym: %s\n", __func__, func_name);
	func = (orange_module_info_get_func_t) dlsym(handle, func_name);
	if (func == NULL) {
		orange_log(ORANGE_LOG_ERR, "loading module '%s' faild: %s\n", module_name, dlerror());
		ret = EINVAL;
		goto exit;
	}

	module_entry->handle = handle;
	module_entry->module = (*func)();
	if (module_entry->module == NULL) {
		ret = EINVAL;
		goto exit;
	}

	ret = __orange_module_sysinit(module_session, module_name, handle, 1);
	if (ret != 0) {
		goto exit;
	}

	module_entry->module->id = module_session->count + 1;
	ret						 = module_entry->module->moduledata->evhand(module_entry->module, ORANGE_MOD_LOAD, NULL);

exit:

	if (ret != 0) {
		dlclose(handle);
		orange_log(ORANGE_LOG_ERR, "load module '%s' faild: %d\n", module_name, ret);
	} else {
		module_session->count++;
	}
	DEBUGP("%s: finished, ret: %d\n", __func__, ret);
	return ret;
}

static int __orange_module_load(struct orange_module_session* module_session, char* module_name, int depended,
								struct orange_module_depend_link* module_depend_link)
{
	int							ret = EINVAL;
	struct orange_module_entry* module_entry;

	DEBUGP("%s: path: %s, depended: %d, module_name: %s, depend_link: %p\n", __func__, module_session->path, depended, module_name, module_depend_link);
#if 0
    TAILQ_FOREACH(module_entry, &module_session->list, entry) {
        if((strlen(module_name) == strlen(module_entry->module->moduledata->name)) &&
            strncmp(module_entry->module->moduledata->name, module_name, strlen(module_name)) == 0) {
            if(depended > 0) {
                ret =__orange_module_depend_link_inc(module_depend_link, module_entry);
                if(ret != 0) {
                    goto exit;
                }

                module_entry->use++;
            }
            ret = EEXIST;
            goto exit;
        }
    }
#endif

	module_entry = orange_zalloc(sizeof(struct orange_module_entry));
	if (module_entry == NULL) {
		ret = ENOMEM;
		goto exit;
	}

	ret = ____orange_module_load(module_session, module_name, module_entry);
	if (ret != 0) {
		orange_free(module_entry);
		goto exit;
	}

	module_entry->use = 1;

	if (depended > 0) {

		ret = __orange_module_depend_link_inc(module_depend_link, module_entry);
		if (ret != 0) {
			orange_free(module_entry);
			goto exit;
		}
		module_entry->use++;
	}

	module_entry->depend_link = module_depend_link;

	TAILQ_INSERT_TAIL(&module_session->list, module_entry, entry);

exit:
	DEBUGP("%s: finished, return: %d\n", __func__, ret);
	return ret;
}

static int __orange_module_load_depend(char* name, uint64_t value, void* data)
{
	int							 ret = 0;
	struct orange_module_params* params;
	// struct orange_module_entry *module_entry;
	char module_name[PATH_MAX];
	char base_string[PATH_MAX];

	params = (struct orange_module_params*) data;
	snprintf(base_string, PATH_MAX, "%s_depend_on_", params->module_name);

	if (strlen(name) > strlen(base_string)) {
		if (strncmp(base_string, name, strlen(base_string)) == 0) {

			memset(module_name, 0, PATH_MAX);
			memcpy(module_name, name + strlen(base_string), strlen(name) - strlen(base_string));

			DEBUGP("%s: path: %s, base_module_name: %s, module_name: %s\n", __func__, params->module_session->path, params->module_name, module_name);

			// ret = __orange_module_load(params->module_session, module_name, 1, params->module_depend_link);
			ret = __orange_module_load_unlock(params->module_session, module_name, 1, params->module_depend_link);
			if (ret == EEXIST) {
				ret = 0;
			}
			DEBUGP("%s: finished, ret = %d\n", __func__, ret);
		}
	}

	return ret;
}

int orange_module_count(struct orange_module_session* module_session)
{
	return module_session == NULL ? 0 : module_session->count;
}

static int __orange_module_load_unlock(struct orange_module_session* module_session, char* module_name, int depended,
									   struct orange_module_depend_link* module_depend_link)
{
	int								  ret;
	char							  path[PATH_MAX];
	struct orange_elf_file*			  elf_file = NULL;
	struct orange_module_params		  params;
	struct orange_module_entry*		  module_entry;
	struct orange_module_depend_link* new_module_depend_link;

	DEBUGP("%s: path: %s, module_name: %s, depended: %d\n", __func__, module_session->path, module_name, depended);

	if (__orange_module_exist(module_session, module_name)) {
		if (depended && module_depend_link != NULL) {

			module_entry = __orange_module_get(module_session, module_name);
			if (module_entry) {
				module_entry->use++;
				ret = __orange_module_depend_link_inc(module_depend_link, module_entry);
			}
		}
		ret = EEXIST;
		goto exit;
	}

	snprintf(path, PATH_MAX, "%s/%s.so", module_session->path, module_name);

	elf_file = orange_elf_open(path);
	if (elf_file == NULL) {
		ret = ENOENT;
		orange_log(ORANGE_LOG_ERR, "load module '%s' faild: %d\n", module_name, ret);
		goto exit;
	}

	/*orange_elf_dump(elf_file, printf); */
	new_module_depend_link = orange_zalloc(sizeof(struct orange_module_depend_link));

	params.module_session	 = module_session;
	params.module_name		  = module_name;
	params.module_depend_link = new_module_depend_link;

	ret = orange_elf_symbol(elf_file, __orange_module_load_depend, &params);
	if (ret == 0) {
		ret = __orange_module_load(module_session, module_name, 0, new_module_depend_link);

		if (ret == 0 && depended != 0 && module_depend_link != NULL) {
			module_entry = __orange_module_get(module_session, module_name);
			if (module_entry) {
				module_entry->use++;
				ret = __orange_module_depend_link_inc(module_depend_link, module_entry);
			}
		}
	}

	if (ret != 0 && depended == 0) {
		__orange_module_depend_link_put(module_depend_link);
	}

exit:

	if (elf_file) {
		orange_elf_close(elf_file);
	}

	DEBUGP("%s: finished, return: %d\n", __func__, ret);
	return ret;
}

int orange_module_load(struct orange_module_session* module_session, char* module_name)
{
	int ret = EINVAL;

	DEBUGP("%s: begin, module_name: %s\n", __func__, module_name);

	if (module_session != NULL) {
		orange_spinlock_lock(&module_session->lock);
		ret = __orange_module_load_unlock(module_session, module_name, 0, NULL);
		orange_spinlock_unlock(&module_session->lock);
	}

	DEBUGP("%s: finished, return: %d\n", __func__, ret);
	return ret;
}

int orange_module_load_all(struct orange_module_session* module_session)
{
	int  ret = EINVAL;
	int  fd;
	char config_filename[PATH_MAX];
	char module_name[PATH_MAX];

	if (module_session == NULL) {
		ret = EINVAL;
		goto exit;
	}

	snprintf(config_filename, PATH_MAX, "%s/%s", module_session->path, ORANGE_MODULE_CONFIG_FILENAME);
	orange_log(ORANGE_LOG_INFO, "trying load module configure: %s\n", config_filename);

	fd = open(config_filename, O_RDONLY);
	if (fd >= 0) {
		char*		config_string;
		struct stat sb;
		char*		begin;
		char*		end;

		if (fstat(fd, &sb) < 0) {
			close(fd);
			goto no_config;
		}

		config_string = (char*) mmap(NULL, sb.st_size, PROT_READ, MAP_SHARED, fd, 0);
		if (config_string == MAP_FAILED) {
			close(fd);
			goto no_config;
		}

		begin = config_string;
		end   = begin;

		while (end < config_string + sb.st_size) {
			while (*end != '\n' && end < config_string + sb.st_size) {
				end++;
			}

			if (*end == '\n' && end >= (config_string + sb.st_size)) {
				break;
			}

			if ((end > begin) && (end - begin) < PATH_MAX) {
				memset(module_name, 0, PATH_MAX);
				strncpy(module_name, begin, end - begin);

				ret = orange_module_load(module_session, module_name);
				if (ret != 0 && ret != EEXIST) {
					goto exit;
				}
			}

			end++;
			begin = end;
		}

		munmap(config_string, sb.st_size);
		close(fd);
		goto exit;
	}

no_config:

exit:
	DEBUGP("%s: finished, return: %d\n", __func__, ret);
	return ret;
}

int orange_module_unload(struct orange_module_session* module_session, char* module_name)
{
	int							ret = ENOENT;
	struct orange_module_entry* module_entry;
	struct orange_module_entry* temp_entry;

	if (module_session == NULL) {
		ret = EINVAL;
		goto exit;
	}

	orange_log(ORANGE_LOG_DEBUG, "try to unloading module: %s\n", module_name);

	orange_spinlock_lock(&module_session->lock);
	DEBUGP("%s: path: %s, module_name: %s\n", __func__, module_session->path, module_name);

	TAILQ_FOREACH_SAFE(module_entry, &module_session->list, entry, temp_entry)
	{
		if ((strlen(module_name) == strlen(module_entry->module->moduledata->mod_name)) &&
			strncmp(module_entry->module->moduledata->mod_name, module_name, strlen(module_name)) == 0) {
			if (module_entry->use != 1) {
				ret = EBUSY;
				goto exit;
			}

			ret = module_entry->module->moduledata->evhand(module_entry->module, ORANGE_MOD_UNLOAD, NULL);
			if (ret != 0) {
				goto exit;
			}

			/* unsysinit */
			ret = __orange_module_sysinit(module_session, module_name, module_entry->handle, 0);
			if (ret != 0) {
				goto exit;
			}

			TAILQ_REMOVE(&module_session->list, module_entry, entry);

			__orange_module_depend_link_put(module_entry->depend_link);

			dlclose(module_entry->handle);
			orange_free(module_entry);

			ret = 0;
			break;
		}
	}

exit:
	if (module_session != NULL) {
		orange_spinlock_unlock(&module_session->lock);
	}

	if (ret != 0) {
		orange_log(ORANGE_LOG_ERR, "unload module '%s' faild, return: %d\n", module_name, ret);
	}

#if 0
    if(orange_log_level_get() >= ORANGE_LOG_DEBUG) {
        orange_module_dump(module_session, orange_log_info);
    }
#endif

	return ret;
}

int orange_module_unload_all(struct orange_module_session* module_session)
{
	int							ret = EINVAL;
	struct orange_module_entry* module_entry;
	struct orange_module_entry* temp_entry;

	DEBUGP("%s: path: %s\n", __func__, module_session->path);

	while (TAILQ_FIRST(&module_session->list) != NULL) {
		TAILQ_FOREACH_REVERSE_SAFE(module_entry, &module_session->list, orange_module_list, entry, temp_entry)
		{
			if (module_entry->use == 1) {
				orange_module_unload(module_session, (char*) (module_entry->module->moduledata->mod_name));
			}
		}
	}

	return ret;
}

void orange_module_dump(struct orange_module_session* module_session, int (*print)(const char* fmt, ...))
{
	struct orange_module_entry* module_entry;

	TAILQ_FOREACH(module_entry, &module_session->list, entry)
	{
		print("name: %s, use: %d\n", module_entry->module->moduledata->mod_name, module_entry->use);
	}
}

struct orange_module_session* orange_module_open(char* path)
{
	struct orange_module_session* module_session = NULL;

	module_session = orange_zalloc(sizeof(struct orange_module_session));
	if (module_session != NULL) {
		strncpy(module_session->path, path, PATH_MAX);
		TAILQ_INIT(&module_session->list);
		orange_spinlock_init(&module_session->lock, "module lock");
	}
	return module_session;
}

void orange_module_close(struct orange_module_session* module_session)
{
	if (module_session == NULL) {
		goto exit;
	}

	orange_module_unload_all(module_session);
	orange_spinlock_destroy(&module_session->lock);
	orange_free(module_session);
exit:
	return;
}

static struct orange_module_session* modules_session = NULL;

int orange_modules_load(char* path)
{
	int ret = 0;

	modules_session = orange_module_open(path);
	if (modules_session == NULL) {
		ret = ENOMEM;
		goto exit;
	}

	ret = orange_module_load_all(modules_session);
	if (ret == 0) {
		orange_log(ORANGE_LOG_INFO, "%d module(s) loaded successful.\n", orange_module_count(modules_session));
		if (orange_log_level_get() >= ORANGE_LOG_DEBUG) {
			orange_module_dump(modules_session, orange_log_info);
		}
	}

exit:
	return ret;
}
