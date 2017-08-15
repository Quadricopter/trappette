#ifndef __MANCHESTER_H__
#define __MANCHESTER_H__

#include <stdint.h>
#include "tsip.h"

/*
 * --------------------------------------------------------------------------
 */

#define BUFFER_SIZE         128


/*
 * --------------------------------------------------------------------------
 */

typedef enum    {

    SIGNALTYPE_NOISE = 0,
    SIGNALTYPE_PREAMBLE,
    SIGNALTYPE_HEADER,
    SIGNALTYPE_DATA
} signaltype_t;

typedef struct  {

    /* New frame available callback*/
    int     (*tsip_cb)(const tsip_t *tsip, void *data);
    void    *tsip_cb_data;

    /* New frame available callback*/
    int     (*stream_cb)(const uint8_t *stream, uint16_t size, void *data);
    void    *stream_cb_data;

    /* Flags */
    uint8_t     verboseLevel;
    uint8_t     lastManchesterBit;
    uint8_t     waitHalfManchesterBit;
    uint8_t     lastDiffBit;

    /* */
    uint8_t     byte;
    int8_t      bitIdx;
    int8_t      bitSkipCount;

    uint8_t     buff[BUFFER_SIZE];
    uint16_t    bufferSize;
    signaltype_t signalType;

    tsip_t      tsip;
}   manchester_t;

/*
 *
 */

void    Manchester_init(manchester_t *ctx);
void    Manchester_setTsipCallback(manchester_t *ctx, int (*tsip_cb)(const tsip_t *tsip, void *data), void *data);
void    Manchester_setStreamCallback(manchester_t *ctx, int (*stream_cb)(const uint8_t *stream, uint16_t size, void *data), void *data);
void    Manchester_dumpBuffer(manchester_t *ctx);
void    Manchester_newHalfBit(manchester_t *ctx, uint8_t bit);

#endif /*__MANCHESTER_H__*/
