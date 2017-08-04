#ifndef __M10_H__
#define __M10_H__

#include "sync.h"
#include "manchester.h"
#include "psk.h"
#include "tsip.h"
#include "filter.h"

typedef enum {

    FILTER_DISABLED = 0,
    FILTER_ENABLED
} FilterMode_t;
/*
 *
 */

typedef struct  m10_s {

    /* */
    sync_t      syncctx;
    psk_t       psk;
    demod_ctx_t demod;

    /* */
    void        *streamCbData;
    
    /* */
    FilterMode_t    filterMode;
    q16_t       qDcLp;
    q16_t       qSigLp;
    int16_t     lastSample;
    int         count;

}   m10_t;

/*
 *
 */

void    M10_init(m10_t *ctx);
void    M10_release(m10_t *ctx);
void    M10_setTsipCallback(m10_t *ctx, int (*tsip_cb)(const tsip_t *tsip, void *data), void *data); 
void    M10_setStreamCallback(m10_t *ctx, int (*stream_cb)(const uint8_t *stream, uint16_t size, void *data), void *data);
void    M10_process16bit48k(m10_t *ctx, int16_t *samples, uint16_t count);
void    M10_setVerboseLevel(m10_t *ctx, uint8_t level);
void    M10_setFilterMode(m10_t *ctx, FilterMode_t filterMode);

#endif /*__M10_H__*/
