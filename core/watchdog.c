#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <assert.h>
#include "watchdog.h"

#ifndef ENABLE_WATCHDOG
#pragma error "ENABLE_WATCHDOG undefined!"
#endif

#define SECONDS_IN_HOUR 3600

void    sprintf_secondsToHMS(char *buff, unsigned int seconds)
{
    unsigned int hour, min, sec;

    buff[0] = 0;
    hour = seconds / SECONDS_IN_HOUR;
    seconds = seconds - (hour * SECONDS_IN_HOUR);
    min = seconds / 60;
    sec = seconds % 60;

    sprintf(buff, "%dh%02dm%02d", hour, min, sec);
}

int Watchdog_init(watchdog_t *wd, void (*signal_cb)(int i))
{
    assert(wd);
    assert(signal_cb);

    memset(wd, 0, sizeof(watchdog_t));

    /*
     *
     */

    if (signal(SIGALRM, signal_cb) == SIG_ERR) {

        fprintf(stderr, "signal failed...\n");
        exit(EXIT_FAILURE);
    }

    /*
     *
     */

    if (timer_create(CLOCK_REALTIME, NULL, &wd->timerid) == -1) {

        fprintf(stderr, "timer_create failed\n");
        exit(EXIT_FAILURE);
    }

    wd->inited = 1;

    return 0;
}

int Watchdog_set(watchdog_t *wd, unsigned int abortSec, unsigned int timeoutSec)
{
    struct itimerspec   timer_value;
    char    buff[32];

    if (!wd || abortSec < 0 || timeoutSec < 0)
        return -1;

    wd->nAbortSecond = abortSec;
    wd->nTimeoutSecond = timeoutSec;

    /*
     *
     */

    if (wd->nAbortSecond > 0) {

        memset(&timer_value, 0, sizeof(struct itimerspec));
        timer_value.it_value.tv_sec = wd->nAbortSecond;
        if (timer_settime(wd->timerid, 0, (const struct itimerspec*) &timer_value, NULL) != 0) {

            fprintf(stderr, "timer_settime failed\n");
            exit(EXIT_FAILURE);
        }

        sprintf_secondsToHMS(buff, wd->nAbortSecond);
        fprintf(stderr, "# Abort in %s, if nothing is received\n", buff);
    }
    if (wd->nTimeoutSecond > 0) {

        sprintf_secondsToHMS(buff, wd->nTimeoutSecond);
        fprintf(stderr, "# Time out set: %s after last received position\n", buff);
    }

    return 0;
}

int Watchdog_kick(watchdog_t *wd)
{
    struct itimerspec   timer_value;

   /*
     * Kill "Abort" timer, we had something!
     */

    if (wd->nAbortSecond > 0) {

//        fprintf(stderr, "# Disable \"Abort\" timer!\n");
        wd->nAbortSecond = 0;
        if (!wd->nTimeoutSecond) {

            Watchdog_delete(wd);
            return 0;
        }                  
    }       

    /*                                                      
     * Time out ( after last received position )
     */
        
    if (wd->nTimeoutSecond > 0) {
        
        memset(&timer_value, 0, sizeof(struct itimerspec));
        timer_value.it_value.tv_sec = wd->nTimeoutSecond;
        if (timer_settime(wd->timerid, 0, (const struct itimerspec*) &timer_value, NULL) != 0) {

            fprintf(stderr, "timer_settime failed\n");
            exit(EXIT_FAILURE);
        }

//        fprintf(stderr, "# Timeout in %d second(s)\n", wd->nTimeoutSecond);
    }

    return 0;
}

int Watchdog_delete(watchdog_t *wd)
{
    if (wd->inited != 0) {

        timer_delete(wd->timerid);
        wd->inited = 0;
    }

    return 0;
}
