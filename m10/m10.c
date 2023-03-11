#include <stdlib.h>
#include <string.h>
#include "m10.h"

#define SIGNAL_LOW_PASS 2
#define DC_CUT_LOW_PASS 7

/*
 *
 */

void    *M10_init(void)
{
    m10_t *ctx = NULL;

    ctx = malloc(sizeof(m10_t));
    memset(ctx, 0, sizeof(m10_t));

    Manchester_init(&ctx->manchester);
    ctx->bEnableFilter = true;

    return ctx;
}

void    M10_release(void *ctx)
{
    free(ctx);
}

void    M10_setStreamCallback(void *ctx, int (*stream_cb)(const uint8_t *stream, uint16_t size, void *data), void *data)
{
    m10_t *m10 = (m10_t*) ctx;

    Manchester_setStreamCallback(&m10->manchester, stream_cb, data);
}

void    M10_setTsipCallback(void *ctx, int (*tsip_cb)(const tsip_t *tsip, void *data), void *data)
{
    m10_t *m10 = (m10_t*) ctx;

    Manchester_setTsipCallback(&m10->manchester, tsip_cb, data);
}

void    M10_process16bit48k(void *ctx, int16_t *samples, uint16_t samplesRead)
{
    int16_t rawSample, newSample;
    int     m;
    m10_t   *m10 = (m10_t*) ctx;

    for (m = 0; m < samplesRead; m++) {

        /*
         * Signal filtering
         */

        rawSample = samples[m];
        newSample = rawSample;

        if (m10->bEnableFilter) {

            low_pass_filter_q16(SIGNAL_LOW_PASS, &m10->qSigLp, rawSample<<16);
            low_pass_filter_q16(DC_CUT_LOW_PASS, &m10->qDcLp,  rawSample<<16);
            newSample = (m10->qSigLp - m10->qDcLp)>>16;       // Remove DC + Lowpass 
        }

        /*
         * Zero-crossing - Clock synchronization
         * 4800 baud @ 48kHz -> 10 samples/bit
         *                   ->  5 samples/half_bit
         * 
         *                        ----      ----           ---- ----           ----      ----
         *   Zero    |||||||||||||    |    |    |         |         |         |    |    |    ||||||||||||||
         * Crossing  |||||||||||||    |    |    |         |         |         |    |    |    ||||||||||||||
         *                             ----      ---- ----           ---- ----      ----
         * Sampling:     NOISE     X    X    X    X    X    X    X    X    X    X    X    X      NOISE
         *                          \    \    \    \    \    \    \    \    \    \    \    \
         *                           ZC+2 ZC+2 ZC+2 ZC+2 \    ZC+2 \    ZC+2 \    ZC+2 ZC+2 ZC+2 
         *                                                ZC+7      ZC+7      ZC+7
         */

        if ((newSample >= 0 && m10->lastSample < 0) ||
            (newSample < 0 && m10->lastSample >= 0)) {

            /* Zero-crossing: reset counter */
            m10->count = 0;
        }
        else if (m10->count == 2 || m10->count == 7) {

            /* 2 or 7 samples after zero-crossing */
            Manchester_newHalfBit(&m10->manchester,
                                  newSample > 0 ? 1 : 0);
        }

        m10->lastSample = newSample;
        m10->count++;
    }
}

void    M10_setVerboseLevel(void *ctx, uint8_t level)
{
    m10_t *m10 = (m10_t*) ctx;

    m10->manchester.verboseLevel = level;
}

void    M10_enableFilter(void *ctx, bool bEnableFilter)
{
    m10_t *m10 = (m10_t*) ctx;

    m10->bEnableFilter = bEnableFilter;
}
