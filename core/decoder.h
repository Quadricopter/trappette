#ifndef __DECODER_H__
#define __DECODER_H__

#include <stdint.h>
#include <stdbool.h>
#include "trappette_sdk.h"

#define DEFAULT_DECODER_EXTENSION   ".so"

typedef struct {

    void  **libs;
    int     count;
} decoder_t;

/*
 *
 */

int     decoder_init(decoder_t *ctx, const char *szPath);
void    decoder_release(decoder_t*);
void    decoder_setDecodedCallback(decoder_t *ctx, int (*decoded_cb)(const decoded_position_t *position, void *data), void *data);
void    decoder_setStreamCallback(decoder_t *ctx, int (*stream_cb)(const uint8_t *stream, uint16_t size, void *data), void *data);
void    decoder_process16bit48k(decoder_t *ctx, int16_t *samples, uint16_t count);
void    decoder_setVerboseLevel(decoder_t *ctx, uint8_t level);
void    decoder_enableFilter(decoder_t *ctx, bool bEnableFilter);

#endif /*__DECODER_H__*/
