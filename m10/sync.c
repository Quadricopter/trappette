#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sync.h"

/*
 * ----------------------------------
 */

int sync_init(sync_t *ctx)
{
    memset(ctx, 0, sizeof(sync_t));
    ctx->syncstat = SYNCSTAT_WAITPREAMBLE;

    return 0;
}

syncstat_t  sync_getState(sync_t *ctx)
{
    return ctx->syncstat;
}

int sync_update(sync_t *ctx, int8_t sign, uint8_t count)
{
    if (count >= 8) {

        switch (ctx->syncstat) {

        case SYNCSTAT_WAITPREAMBLE:
            ctx->synccount = 0;
            ctx->syncstat = SYNCSTAT_HALFSYNC;
            if (ctx->bVerbose)
                fprintf(stderr, "# half sync!\n");
            break;

        case SYNCSTAT_HALFSYNC:
            ctx->synccount++;
            if (ctx->synccount == 50) {

                ctx->syncstat = SYNCSTAT_SYNCHRONIZED;
                if (ctx->bVerbose)
                    fprintf(stderr, "# Fully Synchronized!\n");
                ctx->unsynccount = 0;
            }
            break;

        case SYNCSTAT_SYNCHRONIZED:
            break;

        default:
            fprintf(stderr, "wtf?!?\n");
            exit(EXIT_FAILURE);
        }
    }

    if (count <= 3) {

        ctx->unsynccount++;
        if (ctx->unsynccount >= 10) {

            if (ctx->bVerbose)
                fprintf(stderr, "# Sync lost!\n");
            ctx->syncstat = SYNCSTAT_WAITPREAMBLE;
        }
    }

    if (ctx->bVerbose)
        fprintf(stderr, "# SYNC\t%d\t%d\n", ctx->synccount, ctx->unsynccount);

    return 0;
}
