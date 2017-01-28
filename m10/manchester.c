#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <getopt.h>
#include <signal.h>

#include "manchester.h"
#include "tsip.h"

#define DEFAULT_BYTE_SIZE   8
#define DEFAULT_BIT_SKIP    3

/*
 * --------------------------------------------------------------------------
 */


/*
 *
 */

void    Manchester_init(demod_ctx_t *ctx)
{
    memset(ctx, 0, sizeof(demod_ctx_t));
    ctx->lastManchesterBit = -1;
    ctx->previousFrameType = FRAMETYPE_NOISE_WAITSYNC;
    ctx->lastDiffBit = 1;
    ctx->bitSkipCountTotal = DEFAULT_BIT_SKIP;
}

void    Manchester_setTsipCallback(demod_ctx_t *ctx, int (*tsip_cb)(const tsip_t *tsip, void *data), void *data)
{
    ctx->tsip_cb = tsip_cb;
    ctx->tsip_cb_data = data;
}

void    Manchester_setStreamCallback(demod_ctx_t *ctx, int (*stream_cb)(const uint8_t *stream, uint16_t size, void *data), void *data)
{
    ctx->stream_cb = stream_cb;
    ctx->stream_cb_data = data;
}

void    Manchester_newHalfBit(demod_ctx_t *ctx, uint8_t bit)
{
    if (ctx->verboseLevel > 0)
        fprintf(stderr, "[%04d] %d", ctx->count, bit);

    if (bit == ctx->lastManchesterBit && ctx->waitHalfManchesterBit == 1) {

        // Manchester unsync, skip half period for resync
        if (ctx->verboseLevel > 0)
            fprintf(stderr, " # Sync lost...\n");
//        fprintf(stdout, "\n");
        ctx->waitHalfManchesterBit = 1;
        ctx->byte = 0;
        ctx->bitIdx = 0;
        ctx->bitIdx = DEFAULT_BYTE_SIZE-1;
        ctx->bitSkipCount =  ctx->bitSkipCountTotal;

        /*
         * Frame type
         */

        if (ctx->asciiBufferSize >= 40 &&
            strstr(ctx->asciiBuff, "1010101010101010101010101010101010101010") &&
            ctx->previousFrameType == FRAMETYPE_NOISE_WAITSYNC) {

            if (ctx->verboseLevel > 0)
                fprintf(stderr, "#SYNC\n");
            ctx->previousFrameType = FRAMETYPE_SYNC;
        }
        else if (ctx->asciiBufferSize == 3 &&
                 (!strncmp(ctx->asciiBuff, "101", 3) || !strncmp(ctx->asciiBuff, "010", 3)) &&
                 ctx->previousFrameType == FRAMETYPE_SYNC) {

            if (ctx->verboseLevel > 0)
                fprintf(stderr, "#PREAMBLE\n");
            ctx->previousFrameType = FRAMETYPE_PREAMBLE;
        }
        else if (ctx->asciiBufferSize >= 800 &&
                 ctx->previousFrameType == FRAMETYPE_PREAMBLE) {

            if (ctx->verboseLevel > 0)
                fprintf(stderr, "#DATA\n");
            ctx->previousFrameType = FRAMETYPE_DATA;
        }
        else
            ctx->previousFrameType = FRAMETYPE_NOISE_WAITSYNC;

        /*
         * Frame dump...
         */

        if (ctx->previousFrameType == FRAMETYPE_DATA) {

            // Clean last noise bits
            ctx->buff[TSIP_PACKET_SIZE] = 0;
            ctx->bufferSize = TSIP_PACKET_SIZE;

            if (ctx->stream_cb)
                ctx->stream_cb(ctx->buff, ctx->bufferSize, ctx->stream_cb_data);

            if (TSIP_string2Struct(&ctx->tsip, (const uint8_t*)ctx->buff, ctx->bufferSize) == 0) {

                /*
                 * User callback
                 */

                if (ctx->tsip_cb)
                    ctx->tsip_cb((const tsip_t*)&ctx->tsip, ctx->tsip_cb_data);

                if (ctx->tsip.dAltitude > ctx->dMaxAltitude)
                    ctx->dMaxAltitude = ctx->tsip.dAltitude;
            }
        }

        ctx->asciiBufferSize = 0;
        ctx->bufferSize = 0;
    }
    else {

        if (ctx->waitHalfManchesterBit ==0) {

            // First half period Manchester bit
            ctx->waitHalfManchesterBit = 1;
        }
        else {

            // Second half period Manchester bit
            if (ctx->verboseLevel > 0) {

//                fprintf(stderr, " [%d]", bit);
                fprintf(stderr, " [%d]", bit==ctx->lastDiffBit?1:0);
            }

            ctx->waitHalfManchesterBit = 0;

            /*
             * ASCII buffer
             */

            ctx->asciiBuff[ctx->asciiBufferSize] = bit + '0';
            ctx->asciiBufferSize++;

            /*
             * Binary buffer
             */

            // Differential Manchester: diff = 0, same = 1
            if (bit == ctx->lastDiffBit)
                ctx->byte |= (1<<ctx->bitIdx);
            ctx->lastDiffBit = bit;

            /*
             * Byte full?
             */

            if (ctx->bitIdx == 0) {

                if (ctx->verboseLevel > 0)
                    fprintf(stderr, " --> buff[%03d] = %02X", ctx->bufferSize, ctx->byte);

                ctx->buff[ctx->bufferSize] = ctx->byte;

                // First byte?
                if (!ctx->bufferSize) {

                    // Dirty workaround, clear first BIT
                    ctx->buff[0] = ctx->buff[0] & 0x7F;
                }
                ctx->bufferSize++;
                ctx->bitIdx = DEFAULT_BYTE_SIZE-1;
                ctx->byte = 0;
            }
            else {

                if (ctx->bitSkipCount) {

                    if (ctx->verboseLevel > 0)
                        fprintf(stderr, " skip...\n");
//                    fprintf(stdout, "\n");
                    ctx->bitSkipCount--;
                }
                else
                    ctx->bitIdx--;
            }

        }

        if (ctx->verboseLevel > 0)
            fprintf(stderr, "\n");
    }

    ctx->lastManchesterBit = bit;
    ctx->count++;
}

void    Manchester_newStream(demod_ctx_t *ctx, const uint8_t *stream, uint16_t byteCount)
{
    uint16_t    n;
    uint8_t     bit;

//    fprintf(stderr, "%s(%p, %d)\n", __FUNCTION__, stream, byteCount);

    for (n = 0; n < byteCount; n++) {

        /* Binary stream */
        for (bit = 0; bit < 8; bit++) {

            if (stream[n] & (1<<bit)) {
                Manchester_newHalfBit(ctx, 1);
            }
            else {
                Manchester_newHalfBit(ctx, 0);
            }
        }
    }
}
