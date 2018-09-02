#include <stdio.h>
#include <string.h>

#include "manchester.h"
#include "tsip.h"

#define DEFAULT_BYTE_SIZE   8
#define DEFAULT_BIT_SKIP    3

#define PREAMBLE_MIN_BYTES_LENGTH   3
const char szPreambleStream[PREAMBLE_MIN_BYTES_LENGTH]= { 0xAA, 0xAA, 0xAA };

/*
 * --------------------------------------------------------------------------
 */


/*
 *
 */

void    Manchester_init(manchester_t *ctx)
{
    memset(ctx, 0, sizeof(manchester_t));
    ctx->signalType = SIGNALTYPE_NOISE;
}

void    Manchester_setTsipCallback(manchester_t *ctx, int (*tsip_cb)(const tsip_t *tsip, void *data), void *data)
{
    ctx->tsip_cb = tsip_cb;
    ctx->tsip_cb_data = data;
}

void    Manchester_setStreamCallback(manchester_t *ctx, int (*stream_cb)(const uint8_t *stream, uint16_t size, void *data), void *data)
{
    ctx->stream_cb = stream_cb;
    ctx->stream_cb_data = data;
}

void    Manchester_newHalfBit(manchester_t *ctx, uint8_t bit)
{
    if (ctx->verboseLevel > 0)
        fprintf(stderr, "%d",  bit);

    if (bit == ctx->lastManchesterBit && ctx->waitHalfManchesterBit == 1) {

        // Manchester unsync, skip half period for resync
//        if (ctx->verboseLevel > 0)
//            fprintf(stderr, " # Sync lost...\n");

        /*
         * Signal type
         */

        switch (ctx->signalType) {

            case SIGNALTYPE_NOISE:

                if (ctx->bufferSize >= PREAMBLE_MIN_BYTES_LENGTH &&
                    memcmp(ctx->buff, szPreambleStream, PREAMBLE_MIN_BYTES_LENGTH)) {

                    if (ctx->verboseLevel > 0)
                        fprintf(stderr, "#PREMABLE\n");
                    ctx->signalType = SIGNALTYPE_PREAMBLE;
                }
                break;

            case SIGNALTYPE_PREAMBLE:

                if (ctx->bufferSize == 0 &&
                    ( ctx->byte == 0x80 || ctx->byte == 0x40 ) ){

                    if (ctx->verboseLevel > 0)
                        fprintf(stderr, "#HEADER\n");
                    ctx->signalType = SIGNALTYPE_HEADER;
                }
                break;

            case SIGNALTYPE_HEADER:

                if (ctx->bufferSize >= TSIP_PACKET_SIZE-1) {

                    if (ctx->verboseLevel > 0)
                        fprintf(stderr, "#DATA\n");
                    ctx->signalType = SIGNALTYPE_DATA;
                }
                break;

            case SIGNALTYPE_DATA:

                // Clean last noise bits
                ctx->buff[TSIP_PACKET_SIZE] = 0;
                ctx->bufferSize = TSIP_PACKET_SIZE;

                if (ctx->stream_cb)
                    ctx->stream_cb(ctx->buff, ctx->bufferSize, ctx->stream_cb_data);

                if (TSIP_stream2Struct(&ctx->tsip, (const uint8_t*)ctx->buff, ctx->bufferSize) == 0) {

                    /*
                     * User callback
                     */

                    if (ctx->tsip_cb)
                        ctx->tsip_cb((const tsip_t*)&ctx->tsip, ctx->tsip_cb_data);
                }

                ctx->bufferSize = 0;
                ctx->signalType = SIGNALTYPE_NOISE;
                break;
        }

        ctx->bufferSize = 0;

        ctx->byte = 0;
        ctx->bitIdx = DEFAULT_BYTE_SIZE-1;
        ctx->bitSkipCount = DEFAULT_BIT_SKIP;
    }
    else {

        if (ctx->waitHalfManchesterBit == 0) {

            // First half period Manchester bit
            ctx->waitHalfManchesterBit = 1;
        }
        else {

            ctx->waitHalfManchesterBit = 0;

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

//                if (ctx->verboseLevel > 0)
//                    fprintf(stderr, " --> buff[%03d] = %02X\n", ctx->bufferSize, ctx->byte);

                ctx->buff[ctx->bufferSize] = ctx->byte;

                // First byte?
                if (!ctx->bufferSize) {

                    // Dirty workaround, clear first BIT
                    ctx->buff[0] = ctx->buff[0] & 0x7F;
                }
                ctx->bufferSize++;
                if (ctx->bufferSize >= BUFFER_SIZE) {

                    /* Overflow protection */
                    ctx->signalType = SIGNALTYPE_NOISE;
                    ctx->bufferSize = 0;
                }
                ctx->bitIdx = DEFAULT_BYTE_SIZE-1;
                ctx->byte = 0;
            }
            else {

                ctx->bitIdx--;
                if (ctx->bitSkipCount > 0) {

                    ctx->bitSkipCount--;
                    if (ctx->bitSkipCount == 0)
                        ctx->bitIdx = DEFAULT_BYTE_SIZE - 1;
                }
            }

        }
    }

    ctx->lastManchesterBit = bit;
}

void    Manchester_dumpBuffer(manchester_t *ctx)
{
    uint16_t    n;

    for (n = 0; n < ctx->bufferSize; n++)
        fprintf(stderr, "%02X", ctx->buff[n]);
    fprintf(stderr, "\n");
}
