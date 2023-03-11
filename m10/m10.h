#ifndef __M10_H__
#define __M10_H__

#include "manchester.h"
#include "trappette_sdk.h"
#include "filter.h"


/*
 *
 */

typedef struct  m10_s {

    /* */
    manchester_t    manchester;

    /* */
    bool        bEnableFilter;
    q16_t       qDcLp;
    q16_t       qSigLp;
    int16_t     lastSample;
    int         count;
}   m10_t;

/*
 *
 */

void   *M10_init(void);
void    M10_release(void*);
void    M10_setDecodedCallback(void*, int (*decoded_cb)(const decoded_position_t *tsip, void *data), void *data); 
void    M10_setStreamCallback(void*, int (*stream_cb)(const uint8_t *stream, uint16_t size, void *data), void *data);
void    M10_process16bit48k(void*, int16_t *samples, uint16_t count);
void    M10_setVerboseLevel(void*, uint8_t level);
void    M10_enableFilter(void*, bool bEnable);

#endif /*__M10_H__*/
