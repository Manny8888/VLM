/* Include the standard system header files */

#ifndef _STD_H_
#define _STD_H_

/****************************************************************************/
/* Additional definitions to inform the build */

// caddr_t definition is not picked up
// #define __USE_MISC

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
/****************************************************************************/

#define _THREAD_SAFE

#define OS_LINUX

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>

#include <endian.h>

#include "swapbytes.h"

#include <pthread.h>

typedef void *pthread_addr_t;
typedef void (*pthread_cleanuproutine_t)(void *);
typedef void *(*pthread_startroutine_t)(void *);

#define pthread_yield sched_yield
int pthread_get_expiration_np(const struct timespec *delta, struct timespec *abstime);
int pthread_delay_np(const struct timespec *interval);

#include <stdint.h>
#include <asm/types.h>
#define TRUE 1
#define FALSE 0
#define ESUCCESS 0

#include <limits.h>
/* ---*** TODO: Kludge 'till I figure out how I messed up the toolchain ... */
#ifndef _POSIX_PATH_MAX
#define _POSIX_PATH_MAX 256
#endif
#ifndef _POSIX_ARG_MAX
#define _POSIX_ARG_MAX 4096
#endif

#include <signal.h>

typedef void (*sa_handler_t)(int);
typedef void (*sa_sigaction_t)(int, siginfo_t *, void *);

#include <errno.h>
#include <string.h>

#include <unistd.h>
#include <setjmp.h>
#include <poll.h>
#include <time.h>
#include <sched.h>

#endif
