#ifndef __PSK_H__
#define __PSK_H__

#include <stdint.h>

/*
 *
 */

#define PSK_BYTES_BUFFER_SIZE    128    /* 1024 bits */

typedef struct  psk_s psk_t;

struct  psk_s {

    /* */
    int     (*full_cb)(psk_t *psk, void *data);
    void    *data;

    /* */
    uint8_t pskBuffer[PSK_BYTES_BUFFER_SIZE];
    uint8_t byteIdx;
    uint8_t bitIdx;
};

/*
 *
 */

int     psk_init(psk_t *psk, int (*full_cb)(psk_t *psk, void *data), void *data);
int     psk_addBit(psk_t *psk, uint8_t bit);
void    psk_dump(psk_t *psk);

#endif /*__PSK_H__*/
