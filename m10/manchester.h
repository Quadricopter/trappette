#ifndef __MANCHESTER_H__
#define __MANCHESTER_H__

#include <stdint.h>
#include "tsip.h"

/*
 * --------------------------------------------------------------------------
 */

#define BUFFER_SIZE         2048


/*
 * --------------------------------------------------------------------------
 */

typedef enum    {

    FRAMETYPE_NOISE_WAITSYNC = 0,
    FRAMETYPE_SYNC,
    FRAMETYPE_PREAMBLE,
    FRAMETYPE_DATA
} frametype_t;

typedef struct  {

    /* New frame available callback*/
    int     (*tsip_cb)(const tsip_t *tsip, void *data);
    void    *tsip_cb_data;

    /* New frame available callback*/
    int     (*stream_cb)(const uint8_t *stream, uint16_t size, void *data);
    void    *stream_cb_data;

    /* */
    uint8_t     verboseLevel;
    uint32_t    count;
    int8_t      lastManchesterBit;
    uint8_t     byte;
    int8_t      bitIdx;
    uint8_t     waitHalfManchesterBit;
    uint8_t     bitSkipCountTotal;
    uint8_t     bitSkipCount;
    uint8_t     lastDiffBit;

    uint8_t     buff[BUFFER_SIZE];
    uint16_t    bufferSize;
    char        asciiBuff[BUFFER_SIZE]; // Deprecated!!!
    uint16_t    asciiBufferSize;        // Deprecated!!!
    frametype_t previousFrameType;

    tsip_t      tsip;
    double      dMaxAltitude;
}   demod_ctx_t;

/*
 *
 */

void    Manchester_init(demod_ctx_t *ctx);
void    Manchester_setTsipCallback(demod_ctx_t *ctx, int (*tsip_cb)(const tsip_t *tsip, void *data), void *data);
void    Manchester_setStreamCallback(demod_ctx_t *ctx, int (*stream_cb)(const uint8_t *stream, uint16_t size, void *data), void *data);
void    Manchester_dumpBuffer(demod_ctx_t *ctx);
void    Manchester_newStream(demod_ctx_t *ctx, const uint8_t *stream, uint16_t byteCount);


#endif /*__MANCHESTER_H__*/
