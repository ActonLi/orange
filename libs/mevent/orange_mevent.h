#pragma once

#include "../orange/orange.h"
#include "../orange/orange_module.h"

ORANGE_VERSION_TYPE(orange_mevent);
ORANGE_DEFINE_MODULE_EXTENSION(orange_mevent);


extern int orange_mevent_event_init(void);
extern void orange_mevent_event_fini(void);

extern int orange_mevent_module_init(void);
extern void orange_mevent_module_fini(void);
