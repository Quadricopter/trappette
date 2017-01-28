#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "psk.h"

/*
 *
 */

int psk_init(psk_t *psk, int (*full_cb)(psk_t *psk, void *data), void *data)
{
    if (!psk || !full_cb) {

        fprintf(stderr, "psk_init failed!\n");
        return -1;
    }

    memset(psk, 0, sizeof(psk_t));
    psk->full_cb = full_cb;
    psk->data = data;

    psk->byteIdx = 0;
    psk->bitIdx = 0;

    return 0;
}

int psk_addBit(psk_t *psk, uint8_t bit)
{
    if (!psk)
        return -1;

    if (bit != 0)
        psk->pskBuffer[psk->byteIdx] |= (1<<psk->bitIdx);

    psk->bitIdx++;
    if (psk->bitIdx == 8) {

        psk->bitIdx = 0;
        psk->byteIdx++;
        if (psk->byteIdx == PSK_BYTES_BUFFER_SIZE) {

            psk->full_cb(psk, psk->data);

            memset(psk->pskBuffer, 0, PSK_BYTES_BUFFER_SIZE);
            psk->byteIdx = 0;
            psk->bitIdx = 0;
        }
    }

    return 0;
}

void    psk_dump(psk_t *psk)
{
    uint8_t bit, byte;

    if (!psk) {

        fprintf(stdout, "!psk\n}\n");
        exit(EXIT_FAILURE);
    }

    /* All bytes -1 */
    for (byte = 0; byte < psk->byteIdx; byte++) {

        for (bit = 0; bit < 8; bit++) {

            if (psk->pskBuffer[byte] & (1<<bit))
                fprintf(stdout, "1");
            else
                fprintf(stdout, "0");
        }
    }

    /* Last byte in progress */
    if (psk->bitIdx) {

        for (bit = 0; bit < psk->bitIdx; bit++) {

            if (psk->pskBuffer[psk->byteIdx] & (1<<bit))
                fprintf(stdout, "1");
            else
                fprintf(stdout, "0");
        }
    }
}
