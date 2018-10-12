#ifndef __DAEMON_H_
#define __DAEMON_H_

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#define DAEMON_NAME_SIZE 32
extern void daemon_create(char* daemon_name);
extern int daemon_already_running(char* daemon_name);

#endif
