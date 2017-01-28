#ifndef __SYNC_H__
#define __SYNC_H__

#include <stdint.h>

/*#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <math.h>
*/

/*
 * ----------------------------------
 */

typedef enum {

    SYNCSTAT_WAITPREAMBLE = 0,
    SYNCSTAT_HALFSYNC,
    SYNCSTAT_SYNCHRONIZED
} syncstat_t;

typedef struct {

    syncstat_t  syncstat;
    uint8_t     synccount;
    uint8_t     unsynccount;
    uint8_t     bVerbose;
}   sync_t;

/*
 * ----------------------------------
 */

int         sync_init(sync_t *ctx);
syncstat_t  sync_getState(sync_t *ctx);
int         sync_update(sync_t *ctx, int8_t sign, uint8_t count);


#endif /*__SYNC_H__*/
