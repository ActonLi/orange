#ifndef __ORANGE_MODULE_H__
#define __ORANGE_MODULE_H__

#include "orange.h"

/*
 * Enumerated types for known system startup interfaces.
 *
 * Startup occurs in ascending numeric order; the list entries are
 * sorted prior to attempting startup to guarantee order.  Items
 * of the same level are arbitrated for order based on the 'order'
 * element.
 *
 * These numbers are arbitrary and are chosen ONLY for ordering; the
 * enumeration values are explicit rather than implicit to provide
 * for binary compatibility with inserted elements.
 *
 * The ORANGE_SI_SUB_RUN_SCHEDULER value must have the highest lexical value.
 *
 * The ORANGE_SI_SUB_SWAP values represent a value used by
 * the BSD 4.4Lite but not by FreeBSD; it is maintained in dependent
 * order to support porting.
 *
 * The ORANGE_SI_SUB_PROTO_BEGIN and ORANGE_SI_SUB_PROTO_END bracket a range of
 * initializations to take place at splimp().  This is a historical
 * wart that should be removed -- probably running everything at
 * splimp() until the first init that doesn't want it is the correct
 * fix.  They are currently present to ensure historical behavior.
 */

#define ORANGE_MODULE_NAME_LEN_MAX 512
#define ORANGE_MODULE_PATH_LEN_MAX 2048

enum orange_sysinit_sub_id {
	ORANGE_SI_SUB_DUMMY				   = 0x0000000, /* not executed; for linker*/
	ORANGE_SI_SUB_DONE				   = 0x0000001, /* processed*/
	ORANGE_SI_SUB_TUNABLES			   = 0x0700000, /* establish tunable values */
	ORANGE_SI_SUB_COPYRIGHT			   = 0x0800001, /* first use of console*/
	ORANGE_SI_SUB_SETTINGS			   = 0x0880000, /* check and recheck settings */
	ORANGE_SI_SUB_MTX_POOL_STATIC	  = 0x0900000, /* static mutex pool */
	ORANGE_SI_SUB_LOCKMGR			   = 0x0980000, /* lockmgr locks */
	ORANGE_SI_SUB_VM				   = 0x1000000, /* virtual memory system init*/
	ORANGE_SI_SUB_KMEM				   = 0x1800000, /* kernel memory*/
	ORANGE_SI_SUB_KVM_RSRC			   = 0x1A00000, /* kvm operational limits*/
	ORANGE_SI_SUB_WITNESS			   = 0x1A80000, /* witness initialization */
	ORANGE_SI_SUB_MTX_POOL_DYNAMIC	 = 0x1AC0000, /* dynamic mutex pool */
	ORANGE_SI_SUB_LOCK				   = 0x1B00000, /* various locks */
	ORANGE_SI_SUB_EVENTHANDLER		   = 0x1C00000, /* eventhandler init */
	ORANGE_SI_SUB_VNET_PRELINK		   = 0x1E00000, /* vnet init before modules */
	ORANGE_SI_SUB_KLD				   = 0x2000000, /* KLD and module setup */
	ORANGE_SI_SUB_CPU				   = 0x2100000, /* CPU resource(s)*/
	ORANGE_SI_SUB_RACCT				   = 0x2110000, /* resource accounting */
	ORANGE_SI_SUB_RANDOM			   = 0x2120000, /* random number generator */
	ORANGE_SI_SUB_KDTRACE			   = 0x2140000, /* Kernel dtrace hooks */
	ORANGE_SI_SUB_MAC				   = 0x2180000, /* TrustedBSD MAC subsystem */
	ORANGE_SI_SUB_MAC_POLICY		   = 0x21C0000, /* TrustedBSD MAC policies */
	ORANGE_SI_SUB_MAC_LATE			   = 0x21D0000, /* TrustedBSD MAC subsystem */
	ORANGE_SI_SUB_VNET				   = 0x21E0000, /* vnet 0 */
	ORANGE_SI_SUB_INTRINSIC			   = 0x2200000, /* proc 0*/
	ORANGE_SI_SUB_VM_CONF			   = 0x2300000, /* config VM, set limits*/
	ORANGE_SI_SUB_DDB_SERVICES		   = 0x2380000, /* capture, scripting, etc. */
	ORANGE_SI_SUB_RUN_QUEUE			   = 0x2400000, /* set up run queue*/
	ORANGE_SI_SUB_KTRACE			   = 0x2480000, /* ktrace */
	ORANGE_SI_SUB_OPENSOLARIS		   = 0x2490000, /* OpenSolaris compatibility */
	ORANGE_SI_SUB_CYCLIC			   = 0x24A0000, /* Cyclic timers */
	ORANGE_SI_SUB_AUDIT				   = 0x24C0000, /* audit */
	ORANGE_SI_SUB_CREATE_INIT		   = 0x2500000, /* create init process*/
	ORANGE_SI_SUB_SCHED_IDLE		   = 0x2600000, /* required idle procs */
	ORANGE_SI_SUB_MBUF				   = 0x2700000, /* mbuf subsystem */
	ORANGE_SI_SUB_INTR				   = 0x2800000, /* interrupt threads */
	ORANGE_SI_SUB_SOFTINTR			   = 0x2800001, /* start soft interrupt thread */
	ORANGE_SI_SUB_ACL				   = 0x2900000, /* start for filesystem ACLs */
	ORANGE_SI_SUB_DEVFS				   = 0x2F00000, /* devfs ready for devices */
	ORANGE_SI_SUB_INIT_IF			   = 0x3000000, /* prep for net interfaces */
	ORANGE_SI_SUB_NETGRAPH			   = 0x3010000, /* Let Netgraph initialize */
	ORANGE_SI_SUB_DTRACE			   = 0x3020000, /* DTrace subsystem */
	ORANGE_SI_SUB_DTRACE_PROVIDER	  = 0x3048000, /* DTrace providers */
	ORANGE_SI_SUB_DTRACE_ANON		   = 0x308C000, /* DTrace anon enabling */
	ORANGE_SI_SUB_DRIVERS			   = 0x3100000, /* Let Drivers initialize */
	ORANGE_SI_SUB_CONFIGURE			   = 0x3800000, /* Configure devices */
	ORANGE_SI_SUB_VFS				   = 0x4000000, /* virtual filesystem*/
	ORANGE_SI_SUB_CLOCKS			   = 0x4800000, /* real time and stat clocks*/
	ORANGE_SI_SUB_CLIST				   = 0x5800000, /* clists*/
	ORANGE_SI_SUB_SYSV_SHM			   = 0x6400000, /* System V shared memory*/
	ORANGE_SI_SUB_SYSV_SEM			   = 0x6800000, /* System V semaphores*/
	ORANGE_SI_SUB_SYSV_MSG			   = 0x6C00000, /* System V message queues*/
	ORANGE_SI_SUB_P1003_1B			   = 0x6E00000, /* P1003.1B realtime */
	ORANGE_SI_SUB_PSEUDO			   = 0x7000000, /* pseudo devices*/
	ORANGE_SI_SUB_EXEC				   = 0x7400000, /* execve() handlers */
	ORANGE_SI_SUB_PROTO_BEGIN		   = 0x8000000, /* XXX: set splimp (kludge)*/
	ORANGE_SI_SUB_PROTO_IF			   = 0x8400000, /* interfaces*/
	ORANGE_SI_SUB_PROTO_DOMAININIT	 = 0x8600000, /* domain registration system */
	ORANGE_SI_SUB_PROTO_DOMAIN		   = 0x8800000, /* domains (address families?)*/
	ORANGE_SI_SUB_PROTO_IFATTACHDOMAIN = 0x8800001, /* domain dependent data init*/
	ORANGE_SI_SUB_PROTO_END			   = 0x8ffffff, /* XXX: set splx (kludge)*/
	ORANGE_SI_SUB_KPROF				   = 0x9000000, /* kernel profiling*/
	ORANGE_SI_SUB_KICK_SCHEDULER	   = 0xa000000, /* start the timeout events*/
	ORANGE_SI_SUB_INT_CONFIG_HOOKS	 = 0xa800000, /* Interrupts enabled config */
	ORANGE_SI_SUB_ROOT_CONF			   = 0xb000000, /* Find root devices */
	ORANGE_SI_SUB_DUMP_CONF			   = 0xb200000, /* Find dump devices */
	ORANGE_SI_SUB_RAID				   = 0xb380000, /* Configure GEOM classes */
	ORANGE_SI_SUB_SWAP				   = 0xc000000, /* swap */
	ORANGE_SI_SUB_INTRINSIC_POST	   = 0xd000000, /* proc 0 cleanup*/
	ORANGE_SI_SUB_SYSCALLS			   = 0xd800000, /* register system calls */
	ORANGE_SI_SUB_VNET_DONE			   = 0xdc00000, /* vnet registration complete */
	ORANGE_SI_SUB_KTHREAD_INIT		   = 0xe000000, /* init process*/
	ORANGE_SI_SUB_KTHREAD_PAGE		   = 0xe400000, /* pageout daemon*/
	ORANGE_SI_SUB_KTHREAD_VM		   = 0xe800000, /* vm daemon*/
	ORANGE_SI_SUB_KTHREAD_BUF		   = 0xea00000, /* buffer daemon*/
	ORANGE_SI_SUB_KTHREAD_UPDATE	   = 0xec00000, /* update daemon*/
	ORANGE_SI_SUB_KTHREAD_IDLE		   = 0xee00000, /* idle procs*/
	ORANGE_SI_SUB_SMP				   = 0xf000000, /* start the APs*/
	ORANGE_SI_SUB_RACCTD			   = 0xf100000, /* start raccd*/
	ORANGE_SI_SUB_RUN_SCHEDULER		   = 0xfffffff  /* scheduler*/
};

/*
 * Some enumerated orders; "ANY" sorts last.
 */
enum orange_sysinit_elem_order {
	ORANGE_SI_ORDER_FIRST  = 0x0000000, /* first*/
	ORANGE_SI_ORDER_SECOND = 0x0000001, /* second*/
	ORANGE_SI_ORDER_THIRD  = 0x0000002, /* third*/
	ORANGE_SI_ORDER_FOURTH = 0x0000003, /* fourth*/
	ORANGE_SI_ORDER_MIDDLE = 0x1000000, /* somewhere in the middle */
	ORANGE_SI_ORDER_ANY	= 0xfffffff  /* last*/
};

/*
 * Module dependency declarartion
 */
struct orange_module_depend {
	int ver_minimum;
	int ver_preferred;
	int ver_maximum;
};

/*
 * Module version declaration
 */
struct orange_module_version {
	int version;
};

#define ORANGE_MOD_LOAD 0x01
#define ORANGE_MOD_UNLOAD 0x02

#define ORANGE_MODULE_NAME_MAX 32
#define ORANGE_MODULE_DEPEND_MAX 128

typedef void (*orange_sysinit_nfunc_t)(void*);
typedef void (*orange_sysinit_cfunc_t)(const void*);

typedef struct orange_sysinit {
	enum orange_sysinit_sub_id	 subsystem; /* subsystem identifier*/
	enum orange_sysinit_elem_order order;	 /* init order within subsystem*/
	orange_sysinit_nfunc_t		   func;	  /* function		*/
	void*						   udata;	 /* multiplexer/argument */
} orange_sysinit_t;

typedef struct orange_module* orange_module_t;
typedef int (*orange_modeventhand_t)(orange_module_t, int /* modeventtype_t */, void*);

/*
 * Struct for registering modules statically via SYSINIT.
 */
typedef struct orange_moduledata {
	char*				  mod_name; /* module name */
	orange_modeventhand_t evhand;   /* event handler */
	void*				  priv;		/* extra data */
} orange_moduledata_t;

struct orange_module {
	uint8_t					  id;
	struct orange_sysinit	 sysinit;
	struct orange_moduledata* moduledata;
};

#define ORANGE_DEFINE_MODULE_EXTENSION(name)                                                                                                                   \
	char*   name##_module_name(void);                                                                                                                          \
	uint8_t name##_module_id(void);

#define ORANGE_DECLARE_MODULE(name, data, sub, order)                                                                                                          \
	static struct orange_module __##name##_module = {.sysinit = {sub, order, NULL}, .moduledata = &data};                                                      \
                                                                                                                                                               \
	struct orange_module* name##_module(void);                                                                                                                 \
	struct orange_module* name##_module(void)                                                                                                                  \
	{                                                                                                                                                          \
		return &__##name##_module;                                                                                                                             \
	}

#define ORANGE_DECLARE_MODULE_EXTENSION(name)                                                                                                                  \
	char* name##_module_name(void)                                                                                                                             \
	{                                                                                                                                                          \
		struct orange_module* mod = name##_module();                                                                                                           \
		return mod->moduledata->mod_name;                                                                                                                      \
	}                                                                                                                                                          \
                                                                                                                                                               \
	uint8_t name##_module_id(void)                                                                                                                             \
	{                                                                                                                                                          \
		struct orange_module* mod = name##_module();                                                                                                           \
		return mod->id;                                                                                                                                        \
	}

#define ORANGE_MODULE_VERSION(module, ver)                                                                                                                     \
	static struct orange_module_version __##module##_version = {.version = ver};                                                                               \
                                                                                                                                                               \
	struct orange_module_version* module##_module_version(void);                                                                                               \
	struct orange_module_version* module##_module_version(void)                                                                                                \
	{                                                                                                                                                          \
		return &__##module##_version;                                                                                                                          \
	}

#define ORANGE_MODULE_DEPEND(module, mdepend, vmin, vpref, vmax)                                                                                               \
	static struct orange_module_depend __##module##_depend_on_##mdepend = {.ver_minimum = vmin, .ver_preferred = vpref, .ver_maximum = vmax};                  \
                                                                                                                                                               \
	struct orange_module_depend* module##_depend_on_##mdepend(void);                                                                                           \
	struct orange_module_depend* module##_depend_on_##mdepend(void)                                                                                            \
	{                                                                                                                                                          \
		return &__##module##_depend_on_##mdepend;                                                                                                              \
	}

#define ORANGE_SYSINIT(uniquifier, subsystem, order, func, ident)                                                                                              \
	static struct orange_sysinit sys_init_##uniquifier##_object = {subsystem, order, func, ident};                                                             \
                                                                                                                                                               \
	struct orange_sysinit* sys_init_##uniquifier(void);                                                                                                        \
	struct orange_sysinit* sys_init_##uniquifier(void)                                                                                                         \
	{                                                                                                                                                          \
		return &sys_init_##uniquifier##_object;                                                                                                                \
	}

#define ORANGE_SYSUNINIT(uniquifier, subsystem, order, func, ident)                                                                                            \
	static struct orange_sysinit sys_uninit_##uniquifier##_object = {subsystem, order, func, ident};                                                           \
	struct orange_sysinit*		 sys_uninit_##uniquifier(void);                                                                                                \
	struct orange_sysinit*		 sys_uninit_##uniquifier(void)                                                                                                 \
	{                                                                                                                                                          \
		return &sys_uninit_##uniquifier##_object;                                                                                                              \
	}

struct orange_module_session;

typedef struct orange_module* (*orange_module_info_get_func_t)(void);
typedef struct orange_sysinit* (*orange_module_sysinit_get_func_t)(void);

extern int orange_module_load(struct orange_module_session* module_session, char* module_name);
extern int orange_module_unload(struct orange_module_session* module_session, char* module_name);

extern int orange_module_load_all(struct orange_module_session* module_session);
extern int orange_module_unload_all(struct orange_module_session* module_session);

extern void orange_module_dump(struct orange_module_session* module_session, int (*print)(const char* fmt, ...));

extern int orange_module_count(struct orange_module_session* module_session);
extern struct orange_module_session* orange_module_open(char* path);
extern void orange_module_close(struct orange_module_session* module_session);

extern int orange_modules_load(char* path);

#endif /* !__ORANGE_MODULE_H_ */
