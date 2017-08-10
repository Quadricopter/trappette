#include <string.h>
#include "m10.h"

#define SIGNAL_LOW_PASS 2
#define DC_CUT_LOW_PASS 7

/*
 *
 */

void    M10_init(m10_t *ctx)
{
    memset(ctx, 0, sizeof(m10_t));

    Manchester_init(&ctx->manchester);
    sync_init(&ctx->syncctx);
    ctx->filterMode = FILTER_ENABLED;
}

void    M10_release(m10_t *ctx)
{
}

void    M10_setStreamCallback(m10_t *ctx, int (*stream_cb)(const uint8_t *stream, uint16_t size, void *data), void *data)
{
    Manchester_setStreamCallback(&ctx->manchester, stream_cb, data);
}

void    M10_setTsipCallback(m10_t *ctx, int (*tsip_cb)(const tsip_t *tsip, void *data), void *data)
{
    Manchester_setTsipCallback(&ctx->manchester, tsip_cb, data);
}

void    M10_process16bit48k(m10_t *ctx, int16_t *samples, uint16_t samplesRead)
{
    int16_t rawSample, newSample;
    int     m;

    for (m = 0; m < samplesRead; m++) {

        /*
         * Signal filtering
         */

        rawSample = samples[m];
        newSample = rawSample;

        if (ctx->filterMode != FILTER_DISABLED) {

            low_pass_filter_q16(SIGNAL_LOW_PASS, &ctx->qSigLp, rawSample<<16);
            low_pass_filter_q16(DC_CUT_LOW_PASS, &ctx->qDcLp,  rawSample<<16);
            newSample = (ctx->qSigLp - ctx->qDcLp)>>16;       // Remove DC + Lowpass 
        }

        /*
         * Zero crossing - Clock synchronization
         */

        if (newSample >= 0 && ctx->lastSample < 0) {

            sync_update(&ctx->syncctx, 0, ctx->count);
            ctx->count = 0;
        }

        if (newSample < 0 && ctx->lastSample >= 0) {

            sync_update(&ctx->syncctx, 1, ctx->count);
            ctx->count = 0;
        }

        /*
         * PSK Demodulation
         */

        if (sync_getState(&ctx->syncctx) == SYNCSTAT_SYNCHRONIZED) {

            if (ctx->count == 2 || ctx->count == 7) {

                if (newSample > 0) {

                    Manchester_newHalfBit(&ctx->manchester, 1);
                }
                else {

                    Manchester_newHalfBit(&ctx->manchester, 0);
                }
            }
        }

        ctx->lastSample = newSample;
        ctx->count++;
    }
}

void    M10_setVerboseLevel(m10_t *ctx, uint8_t level)
{
    ctx->manchester.verboseLevel = level;
}

void    M10_setFilterMode(m10_t *ctx, FilterMode_t filterMode)
{
    ctx->filterMode = filterMode;
}
