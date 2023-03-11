#ifndef __TRAPPETTE_SDK_H__
#define __TRAPPETTE_SDK_H__

#include <stdint.h>
#include "tsip.h"

/*
 *
 */

typedef struct {

    const char *szLibName;
    const char *szLibInfo;

    void *(*init)(void);
    void (*release)(void *ctx);
    void (*setTsipCallback)(void *ctx, int (*tsip_cb)(const tsip_t *tsip, void *data), void *data); 
    void (*setStreamCallback)(void *ctx, int (*stream_cb)(const uint8_t *stream, uint16_t size, void *data), void *data);
    void (*process16bit48k)(void *ctx, int16_t *samples, uint16_t count);
    void (*setVerboseLevel)(void *ctx, uint8_t level);
    void (*enableFilter)(void *ctx, bool bEnableFilter);
} trappette_lib_t;

/*
 *
 */

trappette_lib_t *get_lib_info(void);

#endif /* __TRAPPETTE_SDK_H__ */