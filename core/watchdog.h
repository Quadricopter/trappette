#ifndef __WATCHDOG_H__
#define __WATCHDOG_H__

#ifndef ENABLE_WATCHDOG
#pragma error "ENABLE_WATCHDOG undefined!"
#endif

#include <time.h>

typedef struct  {

    int             inited;
    timer_t         timerid;
    unsigned int    nAbortSecond;
    unsigned int    nTimeoutSecond;
}   watchdog_t;

int Watchdog_init(watchdog_t *wd, void (*signal_cb)(int i));
int Watchdog_set(watchdog_t*, unsigned int abortSec, unsigned int timeoutSec);
int Watchdog_kick(watchdog_t*);
int Watchdog_delete(watchdog_t*);

#endif /*__WATCHDOG_H__*/
